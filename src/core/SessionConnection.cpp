#include "SessionConnection.h"

#include "socket/InboundConnection.h"
#include "common/Params.h"
#include "utils/Logger.h"
#include "common/Buffer.h"

namespace Interceptor {

  SessionConnection::SessionConnection(ParamsPtr params,
                                       boost::asio::io_service& ioService)
    : m_ioService(ioService),
      m_params(params),
      m_state(CanSend),
      m_iostrand(ioService),
      m_fsstrand(ioService),
      m_readTimer(ioService),
      m_writeTimer(ioService)
  {
  }

  SessionConnection::~SessionConnection()
  {
    LOG_DEBUG("SessionConnection::~SessionConnection()");
  }

  void SessionConnection::initConnection()
  {
    LOG_DEBUG("SessionConnection::initConnection()");
    m_connection = std::make_shared<Network::TcpInboundConnection>(m_ioService);
  }

  void SessionConnection::init(std::function<void()> callback)
  {
    LOG_DEBUG("SessionConnection::init()");
    callback();
  }

  void SessionConnection::asyncReadSome(char* buffer, size_t bytes,
                                        std::function<void(
                                          const boost::system::error_code&, size_t bytesTransferred)> callback)
  {
    startReadTimer();
    m_connection->asyncReadSome(buffer, bytes, m_iostrand.wrap(
                                  std::bind([this, callback] (const boost::system::error_code & error,
    size_t bytesTransferred) {
      stopReadTimer();
      callback(error, bytesTransferred);
    }, std::placeholders::_1, std::placeholders::_2)));
  }

  void SessionConnection::startReadTimer()
  {
    LOG_DEBUG("SessionConnection::startReadTimer()");
    m_state |= Reading;
    LOG_DEBUG("Setting timeout to " << m_params->config()->m_clientTimeout);
    m_readTimer.expires_from_now(boost::posix_time::seconds(
                                   m_params->config()->m_clientTimeout));
    m_readTimer.async_wait
    (m_iostrand.wrap
     (std::bind(&SessionConnection::handleTimeout,
                shared_from_this(),
                ReadTimer,
                std::placeholders::_1)));

  }

  void SessionConnection::startWriteTimer()
  {
    LOG_DEBUG("SessionConnection::startWriteTimer()");
    m_state |= Sending;
    m_writeTimer.expires_from_now(boost::posix_time::seconds(
                                    m_params->config()->m_serverTimeout));
    m_writeTimer.async_wait
    (m_iostrand.wrap
     (std::bind(&SessionConnection::handleTimeout,
                shared_from_this(),
                WriteTimer,
                std::placeholders::_1)));
  }

  void SessionConnection::stopReadTimer()
  {
    LOG_DEBUG("SessionConnection::stopReadTimer()");
    m_state &= ~Reading;
    m_readTimer.cancel();
  }

  void SessionConnection::stopWriteTimer()
  {
    LOG_DEBUG("SessionConnection::stopWriteTimer()");
    m_state &= ~Sending;
    m_writeTimer.cancel();
  }

  void SessionConnection::postReply(BufferPtr buffer)
  {
    LOG_DEBUG("SessionConnection::postReply()");

    if (buffer->flags() & Buffer::InvalidRequest) {
      closeConnection();
    } else {
      // post directly to the strand and not to the ioService to keep ordering
      m_iostrand.post(
        std::bind(&SessionConnection::sendNext,
                  shared_from_this(), buffer));
    }
  }

  void SessionConnection::sendNext(BufferPtr buffer)
  {
    LOG_DEBUG("SessionConnection::sendNext()");
    {
      std::lock_guard<std::mutex> lock(m_buffersMutex);
      m_buffers.push_back(buffer);

      if (m_state & CanSend) {
        auto v = m_buffers.front();
        m_buffers.pop_front();
        sendReply(v);
      }
    }
  }


  void SessionConnection::sendReply(BufferPtr buffer)
  {
    LOG_DEBUG("SessionConnection::sendReply()");

    if (m_connection) {
      startWriteTimer();
      m_state &= ~CanSend;

      if (buffer->flags() & Buffer::HasMore) {
        m_fsstrand.post(std::bind([ = ]() {
          BufferPtr b = buffer->nextCall()();
          sendNext(b);
        }));
      }

#ifdef DUMP_NETWORK

      for (auto& b : buffer->m_buffers) {
        LOG_NETWORK("SessionConnection::SendReplyBuffer() - Buffer: ",
                    std::string(boost::asio::buffer_cast<const char*>(b),
                                boost::asio::buffer_size(b)));
      }

#endif  // DUMP_NETWORK

      m_connection->asyncWrite(buffer->m_buffers, m_iostrand.wrap
                               (std::bind
                                (&SessionConnection::handleTransmissionCompleted,
                                 shared_from_this(),
                                 buffer,
                                 std::placeholders::_1,
                                 std::placeholders::_2)
                               )
                              );
    }
  }

  void SessionConnection::handleTransmissionCompleted(
    BufferPtr buffer,
    const boost::system::error_code& error, size_t bytesTransferred)
  {
    LOG_DEBUG("Session::handleTransmissionCompleted()");
    stopWriteTimer();

    if (!error)  {
      m_state |= CanSend;

      {
        std::lock_guard<std::mutex> lock(m_buffersMutex);

        if (!m_buffers.empty()) {

          auto v = m_buffers.front();
          m_buffers.pop_front();
          sendReply(v);
        }
      }

    } else {
      LOG_ERROR("Could not send reponse " << error.message());
      closeConnection();
    }

    if (buffer->flags() & Buffer::Closing) {
      closeConnection();
    }
  }

  void SessionConnection::closeConnection()
  {
    LOG_DEBUG("SessionConnection::closeConnection()");

    m_iostrand.post(
      std::bind(&SessionConnection::doCloseConnection, shared_from_this()));
  }

  void SessionConnection::doCloseConnection()
  {
    LOG_DEBUG("SessionConnection::doCloseConnection()");

    if (m_state & Closing) {
      LOG_DEBUG("SessionConnection::doCloseConnection() - already closing");
      return;
    }

    m_state |= Closing;
    stopReadTimer();
    stopWriteTimer();
    m_state &= ~CanSend;

    if (m_connection) {
      m_connection->disconnect();
    }

  }

  void SessionConnection::handleTimeout(TimerType timerType,
                                        const boost::system::error_code& error)
  {
    LOG_DEBUG("SessionConnection::handleTimeout()");

    if (error != boost::asio::error::operation_aborted) {
      if ((timerType == ReadTimer) && ((m_state & Reading & Sending)
                                       || m_buffers.size() > 0)) {
        startReadTimer();  // We are writing something on the socket, so start the timer again
      } else {
        closeConnection();
      }
    }
  }

  ParamsPtr SessionConnection::params() const
  {
    return m_params;
  }

  const std::string SessionConnection::ip() const
  {
    return m_connection ? m_connection->ip() : "unknown";
  }

  boost::asio::ip::tcp::socket& SessionConnection::socket() const
  {
    return m_connection->socket();
  }

  boost::asio::io_service& SessionConnection::ioService() const
  {
    return m_ioService;
  }

};

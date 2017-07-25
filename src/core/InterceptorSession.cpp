#include "InterceptorSession.h"

#include "utils/Logger.h"

#include "socket/InboundConnection.h"
#include "http/HttpRequest.h"
#include "http/HttpBuffer.h"
#include "http/HttpReply.h"

#include "cache/generic_cache.h"
#include "common/Params.h"

InterceptorSession::InterceptorSession(Params* params,
                                       boost::asio::io_service& ioService)
  : m_params(params),
    m_ioService(ioService),
    m_iostrand(ioService),
    m_fsstrand(ioService),
    m_readTimer(ioService),
    m_writeTimer(ioService),
    m_state(CanSend)
{
  m_connection = std::make_shared<TcpInboundConnection>(m_ioService);
}

InterceptorSession::~InterceptorSession()
{
  LOG_DEBUG("InterceptorSession::~InterceptorSession()");
}

boost::asio::ip::tcp::socket& InterceptorSession::socket() const
{
  return m_connection->socket();
}

InboundConnectionPtr InterceptorSession::connection() const
{
  return m_connection;
}

Params* InterceptorSession::params() const
{
  return m_params;
}

void InterceptorSession::postReply(HttpBufferPtr buffer)
{
  LOG_DEBUG("InterceptorSession::postReply()");

  if (buffer->flags() & Http::HttpBuffer::InvalidRequest) {
    closeConnection();
  } else {
    m_ioService.post(
      m_iostrand.wrap(
        std::bind(&InterceptorSession::sendNext, shared_from_this(), buffer)));
  }
}

void InterceptorSession::sendNext(HttpBufferPtr buffer)
{
  LOG_DEBUG("InterceptorSession::sendNext()");
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

void InterceptorSession::sendReply(HttpBufferPtr buffer)
{
  LOG_DEBUG("InterceptorSession::sendReply()");

  if (m_connection) {
    startWriteTimer();
    m_state &= ~CanSend;

    if (buffer->flags() & Http::HttpBuffer::HasMore)
      m_ioService.post(
        m_fsstrand.wrap(buffer->nextCall()));

    m_connection->asyncWrite(buffer->m_buffers, m_iostrand.wrap
                             (std::bind
                              (&InterceptorSession::handleTransmissionCompleted,
                               shared_from_this(),
                               buffer,
                               std::placeholders::_1,
                               std::placeholders::_2)
                             )
                            );
  }
}

void InterceptorSession::handleTransmissionCompleted(
  HttpBufferPtr buffer,
  const boost::system::error_code& error, size_t bytesTransferred)
{
  LOG_DEBUG("InterceptorSession::handleTransmissionCompleted()");
  stopWriteTimer();

  if (!error)  {
    LOG_DEBUG("Response sent ");
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

  if (buffer->flags() & Http::HttpBuffer::Closing) {
    closeConnection();
  }
}

void InterceptorSession::closeConnection()
{
  LOG_DEBUG("InterceptorSession::closeConnection()");

  if (m_state & Closing) {
    return;
  }

  m_state |= Closing;
  stopReadTimer();
  stopWriteTimer();
  m_state &= ~CanSend;
  m_ioService.post(
    m_iostrand.wrap(
      std::bind(&InterceptorSession::doCloseConnection, shared_from_this())));
}

void InterceptorSession::doCloseConnection()
{
  if (m_connection) {
    m_connection->disconnect();
  }
}

void InterceptorSession::start()
{
  LOG_DEBUG("InterceptorSession::start()");

  // Avoid Slow Loris attacks, close connection if nothing read
  if (!(m_state & Closing) && m_connection) {
    startReadTimer();
    m_ioService.post(
      m_iostrand.wrap(
        std::bind(&InterceptorSession::doRead, shared_from_this())));

  }
}

void InterceptorSession::doRead()
{
  m_connection->asyncReadSome(m_requestBuffer, sizeof(m_requestBuffer),
                              std::bind(&InterceptorSession::handleHttpRequestRead, shared_from_this(),
                                        std::placeholders::_1,
                                        std::placeholders::_2)
                             );

}

void InterceptorSession::handleHttpRequestRead(const boost::system::error_code&
    error, size_t bytesTransferred)
{
  LOG_DEBUG("InterceptorSession::handleHttpRequestRead()");
  stopReadTimer();

  if (!error) {
    LOG_INFO("Request read from " << m_connection->ip());

    // Previous request already done, this is a new request
    if (!m_request || m_request->completed() ) {
      m_request = std::make_shared<Http::HttpRequest>(shared_from_this());
    }

    // Append data to current request
    Http::Code ret =  m_request->appendData(m_requestBuffer, bytesTransferred);

    if (ret != Http::Code::Ok) {
      if (!m_reply) {
        m_reply = std::make_shared<Http::HttpReply>(m_request);
      }

      m_reply->declineRequest(ret);
      return;
    }

    if (!m_request->headersReceived()) {
      start();
    } else  {
      // Complete headers received
      m_reply = std::make_shared<Http::HttpReply>(m_request);
      m_reply->process();
      start();
    }
  } else {
    if (error != boost::asio::error::eof
        && error != boost::asio::error::connection_reset) {
      if (m_connection) {
        LOG_ERROR("Error reading request from " << m_connection->ip());
      } else if ( !(m_state & Closing)) {
        LOG_ERROR("Error reading request");
      }
    }

    closeConnection();
  }
}

void InterceptorSession::startReadTimer()
{
  LOG_DEBUG("InterceptorSession::startReadTimer()");
  m_state |= Reading;
  LOG_DEBUG("Setting timeout to " << m_params->config()->m_clientTimeout);
  m_readTimer.expires_from_now(boost::posix_time::seconds(
                                 m_params->config()->m_clientTimeout));
  m_readTimer.async_wait
  (m_iostrand.wrap
   (std::bind(&InterceptorSession::handleTimeout,
              shared_from_this(),
              ReadTimer,
              std::placeholders::_1)));

}

void InterceptorSession::startWriteTimer()
{
  LOG_DEBUG("InterceptorSession::startWriteTimer()");
  m_state |= Sending;
  m_writeTimer.expires_from_now(boost::posix_time::seconds(
                                  m_params->config()->m_serverTimeout));
  m_writeTimer.async_wait
  (m_iostrand.wrap
   (std::bind(&InterceptorSession::handleTimeout,
              shared_from_this(),
              WriteTimer,
              std::placeholders::_1)));
}

void InterceptorSession::stopReadTimer()
{
  LOG_DEBUG("InterceptorSession::stopReadTimer()");
  m_state &= ~Reading;
  m_readTimer.cancel();
}

void InterceptorSession::stopWriteTimer()
{
  LOG_DEBUG("InterceptorSession::stopWriteTimer()");
  m_state &= ~Sending;
  m_writeTimer.cancel();
}

void InterceptorSession::handleTimeout(TimerType timerType,
                                       const boost::system::error_code& error)
{
  LOG_DEBUG("InterceptorSession::handleTimeout()");

  if (error != boost::asio::error::operation_aborted) {
    if ((timerType == ReadTimer) && ((m_state & Reading & Sending)
                                     || m_buffers.size() > 0)) {
      startReadTimer();  // We are writing something on the socket, so start the timer again
    } else {
      closeConnection();
    }
  }
}

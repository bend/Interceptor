#include "BackendConnector.h"

#include "socket/OutboundConnection.h"
#include "utils/Logger.h"
#include "common/Packet.h"

namespace Interceptor {

  namespace Backends   {

    BackendConnector::BackendConnector(BackendCPtr backend,
                                       boost::asio::io_service& ioService)
      : m_backend(backend),
        m_ioService(ioService),
        m_writestrand(ioService),
        m_readstrand(ioService),
        m_callbackstrand(ioService),
        m_state(0)
    {
      std::memset(m_response, 0, sizeof(m_response));
      m_connection = std::make_shared<OutboundTcpConnection>(m_ioService,
                     m_backend->host, std::to_string(m_backend->port));
    }


    BackendConnector::~BackendConnector()
    {
      LOG_DEBUG("BackendConnector::~BackendConnector()");
    }

    bool BackendConnector::connect()
    {
      LOG_DEBUG("BackendConnector::connect()");
      m_connection->asyncResolve(m_writestrand.wrap(
                                   std::bind(&BackendConnector::handleResolved,
                                             shared_from_this(),
                                             std::placeholders::_1,
                                             std::placeholders::_2)));
      return true;
    }


    void BackendConnector::handleResolved(const boost::system::error_code& error,
                                          boost::asio::ip::tcp::resolver::iterator it)
    {
      if (!error) {
        LOG_INFO("BackendConnector::handleResolved() - Resolved");
        m_connection->setEndpoint(it);
        m_connection->asyncConnect(
          m_writestrand.wrap(
            std::bind(&BackendConnector::handleConnected,
                      shared_from_this(),
                      std::placeholders::_1)));
      } else {
        LOG_ERROR("BackendConnector::handleResolved() - could not resolve" <<
                  error.message());
        m_state |= CanWrite;
        forwardNext(); // we flush the queue
      }
    }

    void BackendConnector::handleConnected(const boost::system::error_code& error)
    {
      m_state |= CanWrite;

      if (!error) {
        LOG_INFO("BackendConnector::handleConnected() - connected");
        m_state |= Connected;
      } else {
        LOG_ERROR("BackendConnector::handleConnected() - could not connect " <<
                  error.message());
      }

      forwardNext(); // we always call forward to flush queue
    }

    void BackendConnector::forward(Packet* packet,
                                   std::function<void(Http::StatusCode)> callback)
    {
      assert(m_callback);
      LOG_DEBUG("BackendConnector::forward()");
      auto pair = std::make_pair(packet, callback);
      m_writestrand.post(
        std::bind(&BackendConnector::doPost,
                  shared_from_this(),
                  pair
                 ));
    }

    void BackendConnector::readReply(
      std::function<void(Http::StatusCode, std::stringstream*)>& callback)
    {
      LOG_DEBUG("BackendConnector::readReply()");
      assert(m_callback);
      m_state |= Reading;
      m_connection->asyncReadSome(m_response, sizeof(m_response),
                                  m_readstrand.wrap(
                                    std::bind(&BackendConnector::handleResponseRead,
                                              shared_from_this(),
                                              std::placeholders::_1,
                                              std::placeholders::_2,
                                              m_callback)));
    }

    void BackendConnector::setReplyCallback(
      std::function<void(Http::StatusCode, std::stringstream*)>& callback)
    {
      LOG_DEBUG("BackendConnector::setReplyCallback()");
      assert(callback);
      m_callback = callback;
    }

    void BackendConnector::doPost(
      std::pair<Packet*, std::function<void(Http::StatusCode)>>&
      data)
    {
      LOG_DEBUG("BackendConnector::doPost()");
      m_outbox.push_back(data);
      forwardNext();
    }

    void BackendConnector::forwardNext()
    {
      LOG_DEBUG("BackendConnector::forwardNext()");

      if ((m_state & CanWrite) && m_outbox.size() > 0) {
        auto p = m_outbox.front();
        m_outbox.pop_front();
        doForward(std::get<0>(p), std::get<1>(p));
      } else if (m_outbox.size() > 0) {
        LOG_DEBUG("BackendConnector::forwardNext(), postponing");
      }
    }

    void BackendConnector::doForward(Packet* packet,
                                     std::function<void(Http::StatusCode)>& callback)
    {
      LOG_DEBUG("BackendConnector::doForward()");
      m_state &= ~CanWrite;
      m_state |= Writing;
      m_connection->asyncWrite(packet->m_data,
                               packet->m_size,
                               m_writestrand.wrap(
                                 std::bind(&BackendConnector::handlePacketForwarded,
                                           shared_from_this(),
                                           std::placeholders::_1,
                                           packet,
                                           callback)));
    }

    void BackendConnector::handlePacketForwarded(const boost::system::error_code&
        error, Packet* packet,
        std::function<void(Http::StatusCode)>& callback)
    {
      LOG_DEBUG("BackendConnector::handlePacketForwarded()");
      m_state |= CanWrite;
      m_state &= ~Writing;
      Http::StatusCode code = Http::convertToHttpCode(error);
      LOG_DEBUG("Return code : " << (int) code << " " << error.message());
      delete packet;
      callback(code);
      forwardNext();

      if (!(m_state & Reading)) {
        readReply(m_callback);
      }
    }

    void BackendConnector::handleResponseRead(const boost::system::error_code&
        error, size_t bytesRead,
        std::function<void(Http::StatusCode, std::stringstream*)>& callback)
    {
      LOG_DEBUG("BackendConnector::handleResponseRead()");
      m_state &= ~Reading;

      assert(m_callback);

      if (error) {
        if (error == boost::asio::error::eof
            || error == boost::asio::error::connection_reset) {
          LOG_DEBUG("BackendConnector::handleResponseRead() - connection closed by peer");
          return;
        }

        LOG_DEBUG("BackendConnector::handleResponseRead() : error - " <<
                  error.message());
        Http::StatusCode code = Http::convertToHttpCode(error);
        m_callback(code, nullptr);
      } else {
        std::stringstream* stream = new std::stringstream();
        stream->write(m_response, bytesRead);
        LOG_NETWORK("BackendConnector::handleResponseRead() - got : ",  stream->str() );
        m_callbackstrand.post(std::bind(m_callback, Http::StatusCode::Ok, stream));
        readReply(m_callback);
      }
    }

    const std::string& BackendConnector::name() const
    {
      return m_backend->name;
    }

    void BackendConnector::reset()
    {
      m_connection->disconnect();
    }

  }

}

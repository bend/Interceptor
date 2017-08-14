#include "BackendConnector.h"

#include "socket/OutboundConnection.h"
#include "utils/Logger.h"

namespace Interceptor {

  BackendConnector::BackendConnector(BackendCPtr backend,
                                     boost::asio::io_service& ioService)
    : m_backend(backend),
      m_ioService(ioService),
      m_strand(ioService),
      m_state(0)
  {
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
    m_connection->asyncResolve(m_strand.wrap(
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
        m_strand.wrap(
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

  void BackendConnector::forward(Packet& packet,
                                 std::function<void(Http::Code)> callback)
  {
    LOG_DEBUG("BackendConnector::forward()");
    m_ioService.post(
      m_strand.wrap(
        std::bind(&BackendConnector::doPost,
                  shared_from_this(),
                  std::make_pair(packet, callback)
                 )));
  }

  void BackendConnector::readReply(
    std::function<void(Http::Code, std::stringstream*)> callback)
  {
    LOG_DEBUG("BackendConnector::readReply()");
    m_connection->asyncRead(m_response, sizeof(m_response),
                            m_strand.wrap(
                              std::bind(&BackendConnector::handleResponseRead,
                                        shared_from_this(),
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        callback)));
  }

  void BackendConnector::doPost(std::pair<Packet, std::function<void(Http::Code)>>
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
      LOG_DEBUG("BackendConnector::forwardNext(), forwarding");
      doForward(std::get<0>(p), std::get<1>(p));
    } else if (m_outbox.size() > 0) {
      LOG_DEBUG("BackendConnector::forwardNext(), postponing");
    }
  }

  void BackendConnector::doForward(Packet& packet,
                                   std::function<void(Http::Code)> callback)
  {
    LOG_DEBUG("BackendConnector::doForward()");
    m_state &= ~CanWrite;
    m_connection->asyncWrite(std::get<const char*>(packet),
                             std::get<size_t>(packet),
                             m_strand.wrap(
                               std::bind(&BackendConnector::handlePacketForwarded, shared_from_this(),
                                         std::placeholders::_1, callback)));
  }

  void BackendConnector::handlePacketForwarded(const boost::system::error_code&
      error,
      std::function<void(Http::Code)> callback)
  {
    LOG_DEBUG("BackendConnector::handlePacketForwarded()");
    m_state |= CanWrite;
    Http::Code code = Http::convertToHttpCode(error);
    LOG_DEBUG("Return code : " << (int) code << " " << error.message());
    callback(code);
    forwardNext();
  }

  void BackendConnector::handleResponseRead(const boost::system::error_code&
      error, size_t bytesRead,
      std::function<void(Http::Code, std::stringstream*)> callback)
  {
    LOG_DEBUG("BackendConnector::handleResponseRead()");

    if (error) {
      LOG_DEBUG("BackendConnector::handleResponseRead() : error - " <<
                error.message());
      Http::Code code = Http::convertToHttpCode(error);
      callback(code, nullptr);
    } else {

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

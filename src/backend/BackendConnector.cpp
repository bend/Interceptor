#include "BackendConnector.h"

#include "socket/OutboundConnection.h"
#include "utils/Logger.h"

BackendConnector::BackendConnector(BackendCPtr backend,
                                   boost::asio::io_service& ioService)
  : m_backend(backend),
    m_ioService(ioService)
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
  m_connection->asyncResolve(std::bind(&BackendConnector::handleResolved,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2));
  return true;
}


void BackendConnector::handleResolved(const boost::system::error_code& error,
                                      boost::asio::ip::tcp::resolver::iterator it)
{
  if (!error) {
    LOG_INFO("BackendConnector::handleResolved() - Resolved");
    m_connection->setEndpoint(it);
    m_connection->asyncConnect(std::bind(&BackendConnector::handleConnected,
                                         shared_from_this(),
                                         std::placeholders::_1));
  } else {
    LOG_ERROR("BackendConnector::handleResolved() - could not resolve" <<
              error.message());
  }
}

void BackendConnector::handleConnected(const boost::system::error_code& error)
{
  if (!error) {
    LOG_INFO("BackendConnector::handleConnected() - connected");
  } else {
    LOG_ERROR("BackendConnector::handleConnected() - could not connect " <<
              error.message());
  }
}

void BackendConnector::forward(const char* data, size_t size,
                               std::function<void(Http::Code)> callback)
{
  LOG_DEBUG("BackendConnector::forward()");
  m_connection->asyncWrite(data, size,
  std::bind([ = ](const boost::system::error_code & error) {
    Http::Code code = Http::convertToHttpCode(error);
    callback(code);
  }, std::placeholders::_1));
}

void BackendConnector::handleResponseRead(const boost::system::error_code&
    error, size_t bytesRead,
    std::function<void(Http::Code, std::stringstream&)> callback)
{
  LOG_DEBUG("BackendConnector::handleResponseRead()");

  if (error) {
    LOG_DEBUG("BackendConnector::handleResponseRead() : error - " <<
              error.message());
    std::stringstream stream;
    Http::Code code = Http::convertToHttpCode(error);
    callback(code, stream);
  } else {

  }
}

const std::string& BackendConnector::name() const
{
  return m_backend->name;
}

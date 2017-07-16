#include "BackendConnector.h"

#include "socket/OutboundConnection.h"
#include "utils/Logger.h"

#include <boost/bind.hpp>


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
  m_connection->asyncResolve(boost::bind(&BackendConnector::handleResolved,
                                         shared_from_this(), boost::asio::placeholders::error,
                                         boost::asio::placeholders::iterator));
  return true;
}


void BackendConnector::handleResolved(const boost::system::error_code& error,
                                      boost::asio::ip::tcp::resolver::iterator it)
{
  if (!error) {
    LOG_INFO("BackendConnector::handleResolved() - Resolved");
    m_connection->setEndpoint(it);
    m_connection->asyncConnect(boost::bind(&BackendConnector::handleConnected,
                                           shared_from_this(), boost::asio::placeholders::error));
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

const std::string& BackendConnector::name() const
{
  return m_backend->name;
}

#include "BackendConnector.h"

#include "socket/OutboundConnection.h"
#include "utils/Logger.h"


BackendConnector::BackendConnector(BackendCPtr backend,
                                   boost::asio::io_service& ioService)
  : m_backend(backend),
    m_ioService(ioService)
{}

BackendConnector::~BackendConnector()
{
  LOG_DEBUG("BackendConnector::~BackendConnector()");
}

bool BackendConnector::connect()
{
  LOG_DEBUG("BackendConnector::connect()");
  m_connection = std::make_shared<OutboundTcpConnection>(m_ioService,
                 m_backend->host, std::to_string(m_backend->port));
  return true;
}

const std::string& BackendConnector::name() const
{
  return m_backend->name;
}

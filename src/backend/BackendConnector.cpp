#include "BackendConnector.h"

#include "socket/OutboundConnection.h"


BackendConnector::BackendConnector(const Backend& backend,
                                   boost::asio::io_service& ioService)
  : m_backend(backend),
    m_ioService(ioService)
{}

bool BackendConnector::connect()
{
  m_connection = std::make_shared<OutboundTcpConnection>(m_ioService,
                 m_backend.host, std::to_string(m_backend.port));
  return true;
}

#include "BackendGateway.h"
#include "utils/Logger.h"

BackendGateway::BackendGateway(HttpRequestPtr request)
  : AbstractGateway(request)
{}

void BackendGateway::setConnection(AbstractConnectorPtr connection)
{
  LOG_DEBUG("BackendGateway::setConnection()");
  assert(connection);
  m_connection = std::move(connection);
  m_connection->connect();
}

AbstractConnectorPtr BackendGateway::takeConnection()
{
  LOG_DEBUG("BackendGateway::takeConnection()");
  assert(m_connection);
  return std::move(m_connection);
}

void BackendGateway::handleRequest(
  std::function<void(Http::Code, std::stringstream&)> func)
{
  //m_connection->write(request);
}

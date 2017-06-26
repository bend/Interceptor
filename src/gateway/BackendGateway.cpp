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
}

AbstractConnectorPtr BackendGateway::takeConnection()
{
  LOG_DEBUG("BackendGateway::takeConnection()");
  assert(m_connection);
  return std::move(m_connection);
}

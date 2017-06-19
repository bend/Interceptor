#include "BackendsPool.h"
#include "BackendConnector.h"
#include "utils/Logger.h"

BackendsPool::BackendsPool(boost::asio::io_service& ioService)
  : m_ioService(ioService)
{}

bool BackendsPool::initPool(std::vector<Backend>& backends)
{
  LOG_DEBUG("BackendsPool::initPool()");

  for (auto& backend : backends) {
    BackendConnection connector = std::make_shared<BackendConnector>(backend,
                                  m_ioService);
    connector->connect();
    putConnection(connector);
  }

  return true;
}

BackendConnection BackendsPool::takeConnection(const std::string& backendName)
{
  LOG_DEBUG("BackendsPool::takeConnection()");

  if (m_connections.count(backendName)) {
    auto& connections = m_connections.at(backendName);
    auto& connection = connections.front();
    connections.pop_front();
    return connection;
  }

  return {};
}

void BackendsPool::putConnection(BackendConnection connection)
{
  LOG_DEBUG("BackendsPool::putConnection()");

  if (!m_connections.count(connection->name())) {
    m_connections[connection->name()] = {};
  }

  auto& connections = m_connections.at(connection->name());
  connections.push_back(connection);
}

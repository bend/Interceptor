#include "BackendsPool.h"
#include "BackendConnector.h"
#include "utils/Logger.h"

BackendsPool::BackendsPool(boost::asio::io_service& ioService)
  : m_ioService(ioService)
{}

bool BackendsPool::initPool(std::vector<BackendCPtr>& backends)
{
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  LOG_DEBUG("BackendsPool::initPool()");

  for (auto& backend : backends) {
    initConnection(backend);
  }

  return true;
}

void BackendsPool::initConnection(BackendCPtr backend)
{
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  AbstractConnectorPtr connector = std::make_unique<BackendConnector>(backend,
                                   m_ioService);
  m_backends[backend->name] = backend;
  connector->connect();
  putConnection(std::move(connector));
}

AbstractConnectorPtr BackendsPool::takeConnection(const std::string&
    backendName)
{
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  LOG_DEBUG("BackendsPool::takeConnection()");

  if (m_connections.count(backendName)) {
    auto& connections = m_connections.at(backendName);

    if (connections.size() == 0)  {
      initConnection(m_backends[backendName]);
    }

    LOG_DEBUG("BP : takeConnection : " << m_connections.at(
                backendName).front()->name());
    auto connection = std::move(connections.front());
    connections.pop_front();
    return connection;
  }

  return {};
}

void BackendsPool::putConnection(AbstractConnectorPtr connection)
{
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  LOG_DEBUG("BackendsPool::putConnection()");
  LOG_DEBUG(connection->name());

  if (m_connections.count(connection->name()) == 0) {
    m_connections[connection->name()] = std::list<AbstractConnectorPtr>();
  }

  std::list<AbstractConnectorPtr>& connections = m_connections.at(
        connection->name());
  connections.push_back(std::move(connection));
  LOG_DEBUG("BackendsPool::put connection " <<
            m_connections.at("backend_test").front()->name());
}

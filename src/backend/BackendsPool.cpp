#include "BackendsPool.h"
#include "BackendConnector.h"

bool BackendsPool::initPool(std::vector<Backend>& backends)
{
  for(auto& backend : backends)
  {
	BackendConnection connector = std::make_shared<BackendConnector>(backend, m_ioService);
	connector->connect();
	putConnection(connector);
  }
  return true;
}

BackendConnection BackendsPool::takeConnection(const std::string& backendName)
{
  if(m_connections.count(backendName)) {
	auto& connections = m_connections.at(backendName);
	auto& connection = connections.front();
	connections.pop_front();
	return connection;
  }
  return {};
}

void BackendsPool::putConnection(BackendConnection connection)
{

  if(!m_connections.count(connection->name())) {
	m_connections[connection->name()] = {};
  }

  auto& connections = m_connections.at(connection->name());
  connections.push_back(connection);
}

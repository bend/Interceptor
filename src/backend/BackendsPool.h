#ifndef BACKENDS_POOL_H__
#define BACKENDS_POOL_H__

#include <memory>
#include <vector>
#include <map>
#include <list>
#include <boost/asio/io_service.hpp>

class BackendConnector;
class Backend;

typedef std::shared_ptr<BackendConnector> BackendConnection;

class BackendsPool {
public:
  BackendsPool(boost::asio::io_service& ioService);

  bool initPool(std::vector<Backend>& backends);

  BackendConnection takeConnection(const std::string& backendName);

  void putConnection(BackendConnection connection);

private:
  boost::asio::io_service& m_ioService;
  typedef std::map<std::string, std::list<BackendConnection>> ConnectionsMap;
  ConnectionsMap m_connections;



};


#endif // BACKENDS_POOL_H__

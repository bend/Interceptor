#ifndef BACKENDS_POOL_H__
#define BACKENDS_POOL_H__

#include "gateway/AbstractGateway.h"

#include <memory>
#include <vector>
#include <map>
#include <list>
#include <mutex>
#include <boost/asio/io_service.hpp>

namespace Interceptor {

  class BackendConnector;

  class BackendsPool {
  public:
    BackendsPool(boost::asio::io_service& ioService);

    bool initPool(std::vector<BackendCPtr>& backends);

    AbstractConnectorPtr takeConnection(const std::string& backendName);

    void putConnection(AbstractConnectorPtr connection);

  private:
    void initConnection(BackendCPtr backend);

  private:
    boost::asio::io_service& m_ioService;
    typedef std::map<std::string, std::list<AbstractConnectorPtr>> ConnectionsMap;
    ConnectionsMap m_connections;
    std::map<std::string, BackendCPtr> m_backends;
    std::recursive_mutex m_mutex;

  };

}


#endif // BACKENDS_POOL_H__

#ifndef MAIN_H__
#define MAIN_H__

#include <memory>
#include <future>
#include <list>
#include <boost/asio/io_service.hpp>
#include "vars.h"

namespace Interceptor {

  class Config;
  class Server;

  namespace Cache {
    class Subject;
    class AbstractCacheHandler;
    class CacheMonitor;
  }

  namespace Backends {
    class BackendsPool;
  }

  namespace Modules {
    class ModulesLoader;
  }

  namespace Authentication {
	class AuthenticationLoader;
  }

  class Main {
  public:
    Main();
    ~Main();
    bool init(int argc, char** argv);
    bool reinit();
    void run();
    void stop();

  private:
    bool parsePO(int argc, char** argv);
    bool initCache();
    bool initBackendsPool();
    bool initModules();
	bool initAuthentication();

  private:
    boost::asio::io_service m_ioService;
    Cache::AbstractCacheHandler* m_cacheHandler;
#ifdef ENABLE_LOCAL_CACHE
    Cache::Subject* m_subject;
    Cache::CacheMonitor* m_monitor;
#endif // ENABLE_LOCAL_CACHE
    Config* m_config;
    Backends::BackendsPool* m_pool;
    Modules::ModulesLoader* m_modules;
	Authentication::AuthenticationLoader* m_auths;

    uint16_t m_nbThreads;
    std::vector<std::future<void>> m_futures;
    typedef std::shared_ptr<Server> ServerPtr;
    std::list<ServerPtr> m_servers;
  };

}

#endif // MAIN_H__

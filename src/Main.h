#ifndef MAIN_H__
#define MAIN_H__

#include <memory>
#include <future>
#include <list>
#include <boost/asio/io_service.hpp>
#include "vars.h"

namespace Interceptor {

  class AbstractCacheHandler;
  class Config;
  class Subject;
  class CacheMonitor;
  class BackendsPool;
  class Server;

  class Main {
  public:
    Main();
    ~Main();
    bool init(int argc, char** argv);
    bool reinit();
    void run();
    void stop();

  public:
    int _argc;
    char** _argv;

  private:

    bool parsePO(int argc, char** argv);
    bool initCache();
    bool initBackendsPool();

  private:
    boost::asio::io_service m_ioService;
    AbstractCacheHandler* m_cacheHandler;
#ifdef ENABLE_LOCAL_CACHE
    Subject* m_subject;
    CacheMonitor* m_monitor;
#endif // ENABLE_LOCAL_CACHE
    Config* m_config;
    BackendsPool* m_pool;
    uint16_t m_nbThreads;
    std::vector<std::future<void>> m_futures;
	typedef std::shared_ptr<Server> ServerPtr;
	std::list<ServerPtr> m_servers;
  };

}

#endif // MAIN_H__

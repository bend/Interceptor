#ifndef MAIN_H__
#define MAIN_H__

#include <memory>
#include <boost/asio/io_service.hpp>
#include "vars.h"

class AbstractCacheHandler;
class Config;
class Subject;
class CacheMonitor;
class BackendsPool;

class Main {
public:
  Main();
  ~Main();
  bool init(int argc, char** argv);
  void run();
  void stop();

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
};

#endif // MAIN_H__

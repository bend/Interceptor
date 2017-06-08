#ifndef MAIN_H__
#define MAIN_H__

#include <memory>
#include "vars.h"

class AbstractCacheHandler;
class Config;
class Subject;
class CacheMonitor;

class Main {
public:
  Main();
  ~Main();
  bool init(int argc, char** argv);
  void run();

private:

  bool parsePO(int argc, char** argv);
  bool initCache();

private:
  AbstractCacheHandler* m_cacheHandler;
#ifdef ENABLE_LOCAL_CACHE
  Subject* m_subject;
  CacheMonitor* m_monitor;
#endif // ENABLE_LOCAL_CACHE
  Config* m_config;
  uint16_t m_nbThreads;
};

#endif // MAIN_H__

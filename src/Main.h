#ifndef MAIN_H__
#define MAIN_H__

#include <memory>

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
  Subject* m_subject;
  CacheMonitor* m_monitor;
  Config* m_config;
  uint16_t m_nbThreads;
};

#endif // MAIN_H__

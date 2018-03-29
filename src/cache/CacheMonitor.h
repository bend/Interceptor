#ifndef CACHE_MONITOR_H__
#define CACHE_MONITOR_H__

#include "Subject.h"

#include <fam.h>
#include <vector>
#include <map>
#include <iostream>
#include <thread>


namespace Interceptor::Cache {

  class CacheMonitor {

  public:

    CacheMonitor(Subject& subject);

    ~CacheMonitor();

    std::thread& start();

    void monitorFile(const std::string& p);

    void monitorDirectory(const std::string& p);

    void cancelMonitor(const std::string& p);

  private:

    bool startCallBack();

    bool doMonitorDirectory(const std::string& p);

    bool doMonitorFile(const std::string& p);

    bool doCancelMonitor(const std::string& p);

    const char* eventName(const int& code);

    bool signal(const std::string& filename, const int& code);

  private:
    std::vector<std::string> m_files;
    std::vector<std::string> m_directories;
    std::vector<std::string> m_cancel;
    std::map<std::string, FAMRequest*> m_requests;
    std::thread* m_runningThread;
    bool m_monitoring;

    FAMConnection* m_fc;
    Subject& m_subject;
  };

}

#endif


#include "CacheMonitor.h"

#include "utils/Logger.h"

#include <boost/filesystem.hpp>

using namespace std::chrono_literals;

CacheMonitor::CacheMonitor(Subject& subject)
  : m_runningThread(nullptr),
    m_monitoring(false),
    m_fc(nullptr),
    m_subject(subject)
{
}

CacheMonitor::~CacheMonitor()
{
  LOG_DEBUG("CacheMonitor::~CacheMonitor()");

  m_monitoring = false;

  if (m_runningThread) {
    m_runningThread->join();
  }

  for (auto& kv :  m_requests) {
    if (FAMCancelMonitor(m_fc, kv.second) < 0) {
      LOG_ERROR("Error while stopping Fam monitor");
    } else {
      LOG_INFO("Fam stopped successfully");
    }

    delete kv.second;
  }

  delete m_fc;
  m_fc = nullptr;

  delete m_runningThread;
}

const char* CacheMonitor::eventName(const int& code)
{
  static const char* famevent[] = {
    "",
    "FAMChanged",
    "FAMDeleted",
    "FAMStartExecuting",
    "FAMStopExecuting",
    "FAMCreated",
    "FAMMoved",
    "FAMAcknowledge",
    "FAMExists",
    "FAMEndExist"
  };

  /* Unknown code */
  if (code < FAMChanged || code > FAMEndExist) {
    LOG_ERROR("CacheMonitor::CacheMonitor() : Unknown event code returned by Gamin "
              << code);
    return "unknown";
  }

  /* Return code */
  return famevent[code];
}

std::thread& CacheMonitor::start()
{
  if (m_monitoring) {
    throw new std::runtime_error("monitoring already active");
  }

  m_monitoring = true;
  /* Launch the threads that will monitor the files and the thread that will check the m_updater var */
  m_runningThread = new std::thread(std::bind(&CacheMonitor::startCallBack,
                                    this));
  return *m_runningThread;
}

bool CacheMonitor::startCallBack()
{
  int fam_fd;
  FAMEvent fe;
  fd_set readfds;
  std::string config;

  m_fc = (FAMConnection*)malloc(sizeof(FAMConnection));

  /* Open fam connection */
  if ((FAMOpen(m_fc)) < 0) {
    LOG_ERROR("CacheMonitor::startCallBack() : Fam open failed");
    return false;
  }

  /* Request monitoring for each program argument */
  for (auto& p : m_files) {
    if (!doMonitorFile(p)) {
      return false;
    }
  }

  m_files.clear();

  for (auto& p : m_directories) {
    if (!doMonitorDirectory(p)) {
      return false;
    }
  }

  m_directories.clear();

  /* Initialize select data structure */
  fam_fd = FAMCONNECTION_GETFD(m_fc);
  FD_ZERO(&readfds);
  FD_SET(fam_fd, &readfds);

  /* Loop forever. */
  while (m_monitoring) {
    /*
      if (select(fam_fd + 1, &readfds,
                 0, 0, 0) < 0) {
        LOG_ERROR("CacheMonitor::startCallBack() : Select failed");
        return false;
      }
    */
    if (FAMPending(m_fc) > 0) {

      if (FD_ISSET(fam_fd, &readfds)) {
        if (FAMNextEvent(m_fc, &fe) < 0) {
          LOG_ERROR("CacheMonitor::startCallBack() : FAMNextEvent Failed");
          exit(1);
        }

        signal(fe.filename,	fe.code);
      }
    }

    for (auto& p : m_files) {
      if (!doMonitorFile(p)) {
        return false;
      }
    }

    m_files.clear();

    for (auto& p : m_directories) {
      if (!doMonitorDirectory(p)) {
        return false;
      }
    }

    m_directories.clear();

    for (auto& p : m_cancel) {
      if (!doCancelMonitor(p)) {
        return false;
      }
    }

    m_cancel.clear();


    std::this_thread::sleep_for(400ms);
  }

  return true;
}


bool CacheMonitor::signal(const std::string& filename, const int& code)
{
  switch (code) {
    case FAMDeleted:
      LOG_DEBUG("CacheMonitor::signal() Change occured on  " << filename <<
                " - Code FamDeleted");
      m_subject.notifyListeners({0x01, filename});
      doCancelMonitor(filename);
      break;

    case FAMChanged:
      LOG_DEBUG("CacheMonitor::signal() Change occured on " << filename <<
                " - Code FamChanged");
      m_subject.notifyListeners({0x01, filename});
      doCancelMonitor(filename);
      break;

    case FAMCreated:
      break;
  }

  return true;
}

void CacheMonitor::monitorFile(const std::string& p)
{
  m_files.push_back(p);
}

void CacheMonitor::monitorDirectory(const std::string& p)
{
  m_directories.push_back(p);
}

void CacheMonitor::cancelMonitor(const std::string& p)
{
  m_cancel.push_back(p);
}

bool CacheMonitor::doMonitorFile(const std::string& p)
{
  LOG_DEBUG("CacheMonitor::doMonitorFile() - adding " << p);
  FAMRequest* fr = (FAMRequest*)malloc(sizeof(FAMRequest));

  if (FAMMonitorFile(m_fc, p.c_str(), fr, 0) >= 0) {
    m_requests[p] = fr;
    return true;
  }

  return false;
}

bool CacheMonitor::doMonitorDirectory(const std::string& p)
{
  FAMRequest* fr = (FAMRequest*)malloc(sizeof(FAMRequest));

  if (FAMMonitorDirectory(m_fc, p.c_str(), fr, 0) >= 0) {
    m_requests[p] = fr;
    return true;
  }

  return false;
}

bool CacheMonitor::doCancelMonitor(const std::string& p)
{
  LOG_DEBUG("CacheMonitor::doCancelMonitor() " << p);

  if (m_requests.count(p)) {
    if (FAMCancelMonitor(m_fc, m_requests.at(p)) >= 0) {
      return true;
    }

    return false;
  }

  return false;
}

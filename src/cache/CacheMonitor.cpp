#include "CacheMonitor.h"

#include "utils/Logger.h"

using namespace std::chrono_literals;

CacheMonitor::CacheMonitor()
{
}

CacheMonitor::~CacheMonitor()
{
  LOG_DEBUG("CacheMonitor::~CacheMonitor()");

  for(auto& kv :  m_requests) 
  {
	if (FAMCancelMonitor(m_fc, kv.second) < 0) {
	  LOG_ERROR("Error while stopping Fam monitor");
	} else {
	  LOG_INFO("Fam stopped successfully");
	}
	delete kv.second;
  }

  delete m_fc;
  m_fc = nullptr;
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

std::thread* CacheMonitor::start()
{
  /* Launch the threads that will monitor the files and the thread that will check the m_updater var */
  std::thread* updater = new std::thread(std::bind(&CacheMonitor::startCallBack,
                                         this));
  return updater; //TODO free
}

bool CacheMonitor::startCallBack()
{
  int fam_fd;
  FAMEvent fe;
  fd_set readfds;
  std::string config;

  m_fc = (FAMConnection*)malloc(sizeof(m_fc));

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
  while (true) {
	LOG_DEBUG("FAM LOOP");
	/*
    if (select(fam_fd + 1, &readfds,
               0, 0, 0) < 0) {
      LOG_ERROR("CacheMonitor::startCallBack() : Select failed");
      return false;
    }
	*/
	if(FAMPending(m_fc) > 0) {

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
}


bool CacheMonitor::signal(const std::string& filename, const int& code)
{
  LOG_DEBUG("file changed " << filename << " code " << code );
  switch (code) {
    case FAMDeleted:
      LOG_DEBUG("CacheMonitor::signal() Change occured on  " << filename <<
                " - Code FamDeleted");
      break;

    case FAMCreated:
      LOG_DEBUG("CacheMonitor::signal() : Change occured on  " << filename <<
                " - Code FamCreated");
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
  FAMRequest* fr = (FAMRequest*)malloc(sizeof(FAMRequest));

  LOG_DEBUG("++ wIILL adding file to monitor " << p );
  if (FAMMonitorFile(m_fc, p.c_str(), fr, 0) >= 0) {
	LOG_DEBUG("++adding file to monitor " << p );
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
  if(m_requests.count(p)) 
  {
	if(FAMCancelMonitor(m_fc, m_requests.at(p)) >= 0)
	  return true;
	return false;
  }
  return false;
}

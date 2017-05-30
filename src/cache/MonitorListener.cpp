#include "MonitorListener.h"

#include "CacheMonitor.h"

MonitorListener::MonitorListener(CacheMonitor* cache)
  : m_monitor(cache)
{
}

void MonitorListener::notify(Event& e)
{
  if (e.event == HandledEvent) {
    m_monitor->monitorFile(e.path);
  }
}

#ifndef MONITOR_LISTENER_H__
#define MONITOR_LISTENER_H__

#include "AbstractListener.h"

class CacheMonitor;

class MonitorListener : public AbstractListener {

public:
  static const int HandledEvent = 0x02;

  MonitorListener(CacheMonitor* cache);

  virtual void notify(Event& e) override;

private:
  CacheMonitor* m_monitor;


};

#endif // CACHE_LISTENER_H__

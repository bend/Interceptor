#ifndef CACHE_LISTENER_H__
#define CACHE_LISTENER_H__

#include "AbstractListener.h"

class CacheHandler;

class CacheListener : public AbstractListener {

public:
  static const int HandledEvent = 0x01;

  CacheListener(CacheHandler* cache);

  virtual void notify(Event& e) override;

private:
  CacheHandler* m_cacheHandler;


};

#endif // CACHE_LISTENER_H__

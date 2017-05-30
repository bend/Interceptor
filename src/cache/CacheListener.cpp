#include "CacheListener.h"

#include "CacheHandler.h"

CacheListener::CacheListener(CacheHandler* cache)
  : m_cacheHandler(cache)
{
}

void CacheListener::notify(Event& e)
{
  if (e.event == HandledEvent) {
    m_cacheHandler->purge(e.path);
  }
}

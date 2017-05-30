#ifndef GENERIC_CACHE_H__
#define GENERIC_CACHE_H__

#include "vars.h"
#include "AbstractCacheHandler.h"

#ifdef ENABLE_LOCAL_CACHE
#include "CacheHandler.h"
#include "CacheMonitor.h"
#include "Subject.h"
#include "AbstractListener.h"
#include "CacheListener.h"
#include "MonitorListener.h"
#else
#include "BasicCacheHandler.h"
#endif // ENABLE_LOCAL_CACHE

#endif // GENERIC_CACHE_H__

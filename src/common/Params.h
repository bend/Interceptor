#ifndef PARAMS_H__
#define PARAMS_H__

#include "core/Config.h"
#include "cache/generic_cache.h"
#include "backend/BackendsPool.h"

namespace Interceptor {

  class Params {
  public:
    Params(Config::ServerConfig* config, AbstractCacheHandler* cache,
           BackendsPool* pool)
      : m_config(config),
        m_cache(cache),
        m_pool(pool)
    {}

    ~Params() = default;

    const Config::ServerConfig* config() const
    {
      return m_config;
    }

    AbstractCacheHandler* cache() const
    {
      return m_cache;
    }

    BackendsPool* backendsPool() const
    {
      return m_pool;
    }

    Config::ServerConfig* m_config;
    AbstractCacheHandler* m_cache;
    BackendsPool* m_pool;
  };

}

#endif

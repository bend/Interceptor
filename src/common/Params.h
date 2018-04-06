#ifndef PARAMS_H__
#define PARAMS_H__

#include "core/Config.h"
#include "cache/generic_cache.h"
#include "backend/BackendsPool.h"
#include "modules/ModulesLoader.h"
#include "authentication/AuthenticationLoader.h"

namespace Interceptor {

  class Params {
  public:
    Params(Config::ServerConfig* config, Cache::AbstractCacheHandler* cache,
           Backends::BackendsPool* pool, Modules::ModulesLoader* modulesLoader,
           Authentication::AuthenticationLoader* authLoader)
      : m_config(config),
        m_cache(cache),
        m_pool(pool),
        m_modules(modulesLoader),
        m_authLoader(authLoader)
    {}

    ~Params() = default;

    const Config::ServerConfig* config() const
    {
      return m_config;
    }

    Cache::AbstractCacheHandler* cache() const
    {
      return m_cache;
    }

    Backends::BackendsPool* backendsPool() const
    {
      return m_pool;
    }

    Modules::ModulesLoader* modules() const
    {
      return m_modules;
    }

    Config::ServerConfig* m_config;
    Cache::AbstractCacheHandler* m_cache;
    Backends::BackendsPool* m_pool;
    Modules::ModulesLoader* m_modules;
    Authentication::AuthenticationLoader* m_authLoader;
  };

}

#endif

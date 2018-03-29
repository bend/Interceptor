#ifndef ABSTRACT_CACHE_HANDLER_H__
#define ABSTRACT_CACHE_HANDLER_H__

#include <string>
#include "http/Http.h"

namespace Interceptor::Cache {

  class AbstractCacheHandler {

  public:
    AbstractCacheHandler(size_t maxCacheSize)
      : m_maxCacheSize(maxCacheSize) {};

    virtual ~AbstractCacheHandler() = default;

    virtual std::string eTag(const std::string& file) = 0;

    virtual std::string lastModified(const std::string& file) = 0;

    virtual bool size(const std::string& file, size_t& bytes) = 0;

    virtual Http::StatusCode read(const std::string& file, std::stringstream& out,
                                  size_t& bytes) = 0;

  protected:
    size_t m_maxCacheSize;

  };

}

#endif // ABSTRACT_CACHE_HANDLER_H__

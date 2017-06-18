#ifndef BASIC_CACHE_HANDLER_H__
#define BASIC_CACHE_HANDLER_H__

#include "AbstractCacheHandler.h"
#include <string>

class BasicCacheHandler : public AbstractCacheHandler {
public:
  BasicCacheHandler();

  virtual std::string eTag(const std::string& file) override;

  virtual std::string lastModified(const std::string& file) override;

  virtual bool size(const std::string& file, size_t& bytes) override;

  virtual Http::Code read(const std::string& file, std::stringstream& out,
                          size_t& bytes) override;

};

#endif // BASIC_CACHE_HANDLER_H__

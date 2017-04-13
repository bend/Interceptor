#include "CacheHandler.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"

#include "vars.h"
#include <tuple>

CacheHandler::CacheHandler(size_t maxCacheSize)
  : AbstractCacheHandler(maxCacheSize)
{
  LOG_INFO("Local Cache enabled, max size " << maxCacheSize);
}

std::string CacheHandler::eTag(const std::string& file)
{
  auto tuple = Http::FileUtils::generateCacheData(file);

  if (std::get<0>(tuple).length() > 0) {
    return std::get<0>(tuple);
  }

  return {};
}

std::string CacheHandler::lastModified(const std::string& file)
{
  auto tuple = Http::FileUtils::generateCacheData(file);

  if (std::get<1>(tuple).length() > 0) {
    return std::get<1>(tuple);
  }

  return {};
}

Http::Code CacheHandler::read(const std::string& file,
                              std::stringstream& stream, size_t& bytes)
{
  return Http::FileUtils::readFile(file, stream, bytes);
}

bool CacheHandler::size(const std::string& file, size_t& bytes)
{
  return Http::FileUtils::fileSize(file, bytes);
}

size_t CacheHandler::cacheSize() const
{
  return 0;
}

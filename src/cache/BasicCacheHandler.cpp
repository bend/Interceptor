#include "BasicCacheHandler.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"

#include <tuple>

BasicCacheHandler::BasicCacheHandler()
  : AbstractCacheHandler(0)
{
  LOG_INFO("Running with no local cache");
}

std::string BasicCacheHandler::eTag(const std::string& file)
{
  auto tuple = Http::FileUtils::generateCacheData(file);

  if (std::get<0>(tuple).length() > 0) {
    return std::get<0>(tuple);
  }

  return {};
}

std::string BasicCacheHandler::lastModified(const std::string& file)
{
  auto tuple = Http::FileUtils::generateCacheData(file);

  if (std::get<1>(tuple).length() > 0) {
    return std::get<1>(tuple);
  }

  return {};
}

bool BasicCacheHandler::size(const std::string& file, size_t& bytes)
{
  return Http::FileUtils::fileSize(file, bytes);
}

Http::Code BasicCacheHandler::read(const std::string& file,
                                   std::stringstream& stream, size_t& bytes)
{
  return Http::FileUtils::readFile(file, stream, bytes);
}

size_t BasicCacheHandler::cacheSize() const
{
  return 0;
}

#include "CacheHandler.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"

#include "vars.h"
#include <tuple>

CacheHandler::CacheHandler(size_t maxCacheSize, Subject& subject)
  : AbstractCacheHandler(maxCacheSize),
    m_subject(subject),
    m_fileDatabase(std::make_unique<FileDatabase>()),
    m_filemedataDatabase(std::make_unique<FileMetadataDatabase>())
{
  LOG_INFO("Local Cache enabled, max size " << maxCacheSize);
}

std::string CacheHandler::eTag(const std::string& file)
{
  std::string eTag = m_filemedataDatabase->eTag(file);

  if (eTag.length() > 0) {
    return eTag;
  }

  auto tuple = Http::FileUtils::generateCacheData(file);

  if (std::get<0>(tuple).length() > 0) {
    eTag = std::get<0>(tuple);
    m_filemedataDatabase->setETag(file, eTag);
    m_filemedataDatabase->setLastModified(file, std::get<1>(tuple));
    m_subject.notifyListeners({0x02, file});
    return eTag;
  }

  return {};
}

std::string CacheHandler::lastModified(const std::string& file)
{
  std::string lm = m_filemedataDatabase->lastModified(file);

  if (lm.length() > 0) {
    return lm;
  }

  if (eTag(file).length() > 0) { // eTag is always the first thing to be generated
    return m_filemedataDatabase->lastModified(file);
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
  int64_t s = m_filemedataDatabase->size(file);

  if (s > -1) {
    bytes = (size_t)s;
    return true;
  }

  if (eTag(file).length() ==
      0) { // eTag is always the first thing to be generated
    return false;
  }

  if (Http::FileUtils::fileSize(file, bytes)) {
    m_filemedataDatabase->setSize(file, bytes);
  }

  return true;
}

void CacheHandler::purge(const std::string& path)
{
  LOG_DEBUG("CacheHandler::purge() - purging " << path);
  m_filemedataDatabase->purge(path);
  m_fileDatabase->purge(path);
}

size_t CacheHandler::cacheSize() const
{
  return 0;
}

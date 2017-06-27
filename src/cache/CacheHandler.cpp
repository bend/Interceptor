#include "CacheHandler.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"

#include "vars.h"
#include <tuple>
#include <iomanip>

CacheHandler::CacheHandler(size_t maxCacheSize, Subject& subject)
  : AbstractCacheHandler(maxCacheSize),
    m_subject(subject),
    m_fileDatabase(std::make_unique<FileDatabase>(maxCacheSize)),
    m_filemedataDatabase(std::make_unique<FileMetadataDatabase>())
{
  LOG_INFO("Local Cache enabled, max size " << maxCacheSize);
}

std::string CacheHandler::eTag(const std::string& file)
{
  std::string eTag = m_filemedataDatabase->eTag(file);

  if (eTag.length() > 0) {
    LOG_DEBUG("CacheHandler::eTag() - cache hit for " << file);
    return eTag;
  }

  auto tuple = FileUtils::generateCacheData(file);

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
    LOG_DEBUG("CacheHandler::lastModified() - cache hit for " << file);
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
  auto data = m_fileDatabase->data(file);

  if (data) {
    LOG_DEBUG("CacheHandler::read() - cache hit for " << file);
    stream.write(reinterpret_cast<const char*>(data->first), data->second);
    return Http::Code::Ok;
  }

  unsigned char* mdata;
  Http::Code ret;
  LOG_DEBUG("CacheHandler::read() - cache miss for " << file);

  if ((ret = FileUtils::readFile(file, &mdata, bytes)) == Http::Code::Ok) {
    stream.write(reinterpret_cast<const char*>(mdata), bytes);
    m_fileDatabase->setData(file, mdata, bytes);
  }

  return ret;
}

bool CacheHandler::size(const std::string& file, size_t& bytes)
{
  int64_t s = m_filemedataDatabase->size(file);

  if (s > -1) {
    LOG_DEBUG("CacheHandler::size() - cache hit for " << file);
    bytes = (size_t)s;
    return true;
  }

  // eTag is always the first thing to be generated
  if (eTag(file).length() == 0) {
    return false;
  }

  if (FileUtils::fileSize(file, bytes)) {
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

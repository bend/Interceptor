#include "FileMetadataDatabase.h"

std::string FileMetadataDatabase::eTag(const std::string& filename) const
{
  if (m_eTagMap.count(filename)) {
    return m_eTagMap.at(filename);
  }

  return {};
}

std::string FileMetadataDatabase::lastModified(const std::string& filename)
const
{
  if (m_lastModifyMap.count(filename)) {
    return m_lastModifyMap.at(filename);
  }

  return {};
}

int64_t FileMetadataDatabase::size(const std::string& filename) const
{
  if (m_sizeMap.count(filename)) {
    return m_sizeMap.at(filename);
  }

  return -1;
}

void FileMetadataDatabase::setETag(const std::string& filename,
                                   const std::string& eTag)
{
  m_eTagMap[filename] = eTag;
}

void FileMetadataDatabase::setLastModified(const std::string& filename,
    const std::string& lm)
{
  m_lastModifyMap[filename] = lm;
}

void FileMetadataDatabase::setSize(const std::string& filename, size_t size)
{
  m_sizeMap[filename] = size;
}

void FileMetadataDatabase::purge(const std::string& path)
{
  m_eTagMap.erase(path);
  m_lastModifyMap.erase(path);
  m_sizeMap.erase(path);
}

#include "FileDatabase.h"

void FileDatabase::setData(const std::string& path, unsigned char* data)
{
  m_dataMap[path] = data;
}

const unsigned char* FileDatabase::data(const std::string& path) const
{
  if (m_dataMap.count(path) > 0) {
    return m_dataMap.at(path);
  }

  return nullptr;
}

void FileDatabase::purge(const std::string& path)
{
  if (m_dataMap.count(path) > 0) {
    delete[] m_dataMap.at(path);
    m_dataMap.erase(path);
  }
}

size_t FileDatabase::size() const
{
  return 0;
}

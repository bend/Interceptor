#include "FileDatabase.h"

#include "utils/Logger.h"

namespace Interceptor {

  FileDatabase::FileDatabase(size_t maxSize)
    : m_maxSize(maxSize),
      m_size(0)
  {}

  bool compare_bufferhit(const std::shared_ptr<FileDatabase::BufferHit> b1,
                         const std::shared_ptr<FileDatabase::BufferHit> b2)
  {
    return b1->second < b2->second;
  }

  void FileDatabase::setData(const std::string& path, char* data,
                             size_t size)
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if (m_dataMap.count(path) > 0) {
      m_size -= m_dataMap.at(path).second;
    }

    m_dataMap[path] = {data, size};
    auto bh = std::make_shared<BufferHit>(path, 1);
    m_hitMap[path] = bh;
    m_hitList.push_back(bh);
    m_size += size;

    if (m_size > m_maxSize) {
      shrinkToFit(m_maxSize);
    }
  }

  const FileDatabase::Buffer* FileDatabase::data(const std::string& path) const
  {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if (m_dataMap.count(path) > 0) {
      m_hitMap[path]->second++;
      return &m_dataMap.at(path);
    }

    return {};
  }

  void FileDatabase::purge(const std::string& path)
  {
    if (m_dataMap.count(path) > 0) {
      m_size -= m_dataMap.at(path).second;
      delete[] m_dataMap.at(path).first;
      m_dataMap.erase(path);
    }
  }

  size_t FileDatabase::size() const
  {
    return m_size;
  }

  void FileDatabase::shrinkToFit(size_t bytes)
  {
    LOG_DEBUG("FileDatabase::shrinkToFit()");
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_hitList.sort(compare_bufferhit);
    LOG_DEBUG("Before - " << m_hitList.size());

    while (m_size > bytes) {
      auto bh = m_hitList.front();
      auto buffer = m_dataMap.at(bh->first);
      m_size -= buffer.second;
      delete[] buffer.first;
      m_hitList.pop_front();
      m_hitMap.erase(bh->first);
      m_dataMap.erase(bh->first);
    }

    LOG_DEBUG("After - " << m_hitList.size());
  }

}

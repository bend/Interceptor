#ifndef FILE_DATABASE_H__
#define FILE_DATABASE_H__

#include "AbstractDatabase.h"

#include <list>
#include <memory>
#include <mutex>

namespace Interceptor {

  class FileDatabase : public AbstractDatabase {
  public:
    typedef std::pair<const char*, size_t> Buffer;
    typedef std::pair<std::string, int64_t> BufferHit;
    FileDatabase(size_t maxSize);
    ~FileDatabase();

    virtual void purge(const std::string& path) override;
    virtual size_t size() const override;

    void setData(const std::string& path, char* data, size_t size);
    const Buffer* data(const std::string& path) const;
    void shrinkToFit(size_t bytes);

  private:
    MetaDataMap<Buffer> m_dataMap;
    mutable std::list<std::shared_ptr<BufferHit>> m_hitList;
    mutable MetaDataMap<std::shared_ptr<BufferHit>> m_hitMap;
    const size_t m_maxSize;
    size_t m_size;
    mutable std::recursive_mutex m_mutex;
  };

}

#endif

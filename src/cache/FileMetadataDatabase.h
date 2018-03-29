#ifndef FILE_METADATADATABASE_H__
#define FILE_METADATADATABASE_H__

#include "AbstractDatabase.h"

namespace Interceptor::Cache {

  class FileMetadataDatabase : public AbstractDatabase {
  public:
    FileMetadataDatabase() = default;

    std::string eTag(const std::string& filename) const;
    std::string lastModified(const std::string& filename) const;
    int64_t size(const std::string& filename) const;

    void setETag(const std::string& filename, const std::string& eTag);
    void setLastModified(const std::string& filename, const std::string& lm);
    void setSize(const std::string& filename, size_t size);
    virtual void purge(const std::string& path) override;
    virtual size_t size() const override;

  private:
    MetaDataMap<std::string> m_eTagMap;
    MetaDataMap<std::string> m_lastModifyMap;
    MetaDataMap<size_t> m_sizeMap;

  };

}

#endif // FILE_METADATADATABASE_H__

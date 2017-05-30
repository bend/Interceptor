#ifndef FILE_METADATADATABASE_H__
#define FILE_METADATADATABASE_H__

#include <map>

class FileMetadataDatabase {
public:
  FileMetadataDatabase() = default;

  std::string eTag(const std::string& filename) const;
  std::string lastModified(const std::string& filename) const;
  int64_t size(const std::string& filename) const;

  void setETag(const std::string& filename, const std::string& eTag);
  void setLastModified(const std::string& filename, const std::string& lm);
  void setSize(const std::string& filename, size_t size);
  void purge(const std::string& path);

private:
  template <class C>
  using MetaDataMap = std::map<std::string, C>;
  MetaDataMap<std::string> m_eTagMap;
  MetaDataMap<std::string> m_lastModifyMap;
  MetaDataMap<size_t> m_sizeMap;

};

#endif // FILE_METADATADATABASE_H__

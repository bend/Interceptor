#ifndef FILE_DATABASE_H__
#define FILE_DATABASE_H__

#include "AbstractDatabase.h"

class FileDatabase : public AbstractDatabase {
public:
  typedef std::pair<const unsigned char*, size_t> Buffer;
  FileDatabase() = default;

  virtual void purge(const std::string& path) override;
  virtual size_t size() const override;

  void setData(const std::string& path, unsigned char* data, size_t size);
  const Buffer* data(const std::string& path) const;

private:
  MetaDataMap<Buffer> m_dataMap;
};

#endif

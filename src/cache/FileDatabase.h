#ifndef FILE_DATABASE_H__
#define FILE_DATABASE_H__

#include "AbstractDatabase.h"

class FileDatabase : public AbstractDatabase {
public:
  FileDatabase() = default;

  virtual void purge(const std::string& path) override;
  virtual size_t size() const override;

  void setData(const std::string& path, unsigned char* data);
  const unsigned char* data(const std::string& path) const;

private:
  MetaDataMap<unsigned char*> m_dataMap;
};

#endif

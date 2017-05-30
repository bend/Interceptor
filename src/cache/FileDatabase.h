#ifndef FILE_DATABASE_H__
#define FILE_DATABASE_H__

#include <string>

class FileDatabase {
public:
  FileDatabase() = default;

  void purge(const std::string& path);
};

#endif

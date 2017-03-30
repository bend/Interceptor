#ifndef UTILS_H__
#define UTILS_H__

#include <string>

class Utils {

public:

  static bool readFile(const std::string& path, unsigned char** data, size_t& bytes);

  static bool readFile(const std::string& path, std::stringstream& stream, size_t& bytes);

  static std::string mimeType(const std::string& path);

  static std::string extension(const std::string& filename);

};

#endif // UTILS_H__

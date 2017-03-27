#ifndef UTILS_H__
#define UTILS_H__

#include <string>

class Utils {

public:

  static bool readFile(const std::string& path, unsigned char** data, size_t& bytes);

  static std::string getMimeType(const std::string& path);

};

#endif // UTILS_H__

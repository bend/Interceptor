#ifndef FILE_UTILS_H__
#define FILE_UTILS_H__

#include <string>
#include <vector>

#include "http/Http.h"

namespace Http {
  namespace FileUtils {

    /**
     * Read the file content into a buffer
     * @return the initialized buffer and the size
     */
    Code readFile(const std::string& path, unsigned char** data,
                  size_t& bytes);

    Code readFile(const std::string& path, std::stringstream& stream,
                  size_t& bytes);

    Code readFile(const std::string& filename, size_t from, size_t to,
                  std::stringstream& stream, size_t& bytesRead);

    Code calculateBounds(const std::string& filename, int64_t& from, int64_t& to);

    bool fileSize(const std::string& path, size_t& bytes);

    bool exists(const std::string& path);

    std::string mimeType(const std::string& path);

    std::string extension(const std::string& filename);

    /**
     * Returns a tuple containing:
     * [0] - Etag
     * [1] - Last modified in GMT format
     */
    std::tuple<std::string, std::string> generateCacheData(
      const std::string& path);

  }

}

#endif // FILE_UTILS_H__

#ifndef FILE_UTILS_H__
#define FILE_UTILS_H__

#include <string>
#include <vector>

namespace FileUtils {

  /**
   * Read the file content into a buffer
   * @return the initialized buffer and the size
   */
  bool readFile(const std::string& path, unsigned char** data,
                size_t& bytes);

  bool readFile(const std::string& path, std::stringstream& stream,
                size_t& bytes);

  /**
   * Read a part of a file into a stringstream
   * Return a vector containing
   * [0] first byte index
   * [1] last byte index
   * [2] total file size on disk
   */
  bool readFile(const std::string& filename,
                const std::tuple<int64_t, int64_t>& bytes, std::stringstream& stream,
                std::vector<uint64_t>& sizes);

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

#endif // FILE_UTILS_H__

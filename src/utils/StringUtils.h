#ifndef STRING_UTILS_H__
#define STRING_UTILS_H__

#include <iostream>

namespace StringUtils {

  /**
   * @brief check if pattern is present in data
   *
   * @param data the data to check against
   * @param pattern the searched pattern
   * @param s the size of data
   *
   * @return true if pattern found, false otherwise
   */
  bool containsString(const unsigned char* data, const std::string& pattern,
                      size_t s);

  /**
   * @brief check if the pattern is present in data and returns the position of the latest character
   *
   * @param data the data to check against
   * @param pattern the searched pattern
   * @param s the size of data
   *
   * @return  the index of the last character of pattern in data if found, -1 otherwise
   */
  int64_t findString(const unsigned char* data, const std::string& pattern,
                     size_t s);

};

#endif // STRING_UTILS_H__

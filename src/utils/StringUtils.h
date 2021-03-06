#ifndef STRING_UTILS_H__
#define STRING_UTILS_H__

#include <iostream>

namespace Interceptor::StringUtils {

  /**
   * @brief check if pattern is present in data
   *
   * @param data the data to check against
   * @param pattern the searched pattern
   * @param s the size of data
   *
   * @return true if pattern found, false otherwise
   */
  bool containsString(const char* data, const std::string& pattern,
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
  int64_t findString(const char* data, const std::string& pattern,
                     size_t s);

  bool regexMatch(const std::string& regex, const std::string& pattern);

};

#endif // STRING_UTILS_H__

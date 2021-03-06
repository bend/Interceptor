#include "StringUtils.h"

#include <regex>

namespace Interceptor::StringUtils {
  bool containsString(const char* data, const std::string& pattern,
                      size_t s)
  {
    return findString(data, pattern, s) > -1;
  }

  int64_t findString(const char* data, const std::string& pattern,
                     size_t s)
  {
    for ( size_t i = 0; i < s; ++i) {
      if (data[i] == pattern[0]) {
        for (size_t j = 1; j < pattern.length(); ++j) {
          if ( data[i + j] != pattern[j]) {
            break;
          }

          if ( j == pattern.length() - 1) {
            return i + j;
          }
        }
      }
    }

    return -1;
  }

  bool regexMatch(const std::string& regex, const std::string& pattern)
  {

    std::regex reg(regex, std::regex_constants::ECMAScript);

    return std::regex_search(pattern, reg);
  }

};

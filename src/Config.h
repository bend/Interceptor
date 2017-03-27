#ifndef CONFIG_H__
#define CONFIG_H__

#include <string>

class Config {
public:
  Config() = default;

  std::string docRoot() const;

};

extern Config SConfig;

#endif //CONFIG_H__

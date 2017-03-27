#include "Config.h"

Config SConfig;

std::string Config::docRoot() const
{
  return "/usr/share/nginx/homepage/";
}

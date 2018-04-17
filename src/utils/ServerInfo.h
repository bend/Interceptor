#ifndef SERVER_H__
#define SERVER_H__

#include <string>

namespace Interceptor::ServerInfo {

  std::string commonName();
  std::string version();
  std::string name();
  std::string osName();
  std::string build();
  std::string currentDate();
  std::string buildDate();
}

#endif //SERVER_H__

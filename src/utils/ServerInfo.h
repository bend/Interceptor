#ifndef SERVER_H__
#define SERVER_H__

#include <string>

#define SERVER_NAME		"Interceptor"
#define SERVER_VERSION_MAJOR	0
#define SERVER_VERSION_MINOR	2
#define SERVER_VERSION_PATCH	18

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

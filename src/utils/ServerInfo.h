#ifndef SERVER_H__
#define SERVER_H__

#include <string>

#define SERVER_NAME		"Interceptor"
#define SERVER_VERSION	"0.1"

namespace Interceptor::ServerInfo {

  std::string commonName();
  std::string version();
  std::string name();
  std::string osName();
  std::string build();
}

#endif //SERVER_H__

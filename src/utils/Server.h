#ifndef SERVER_H__
#define SERVER_H__

#include <string>

#define SERVER_NAME		"Interceptor"
#define SERVER_VERSION	"0.1"

class Server {
public:
  static std::string getCommonName();
  static std::string getVersion();
  static std::string getName();
  static std::string getOsName();
};

#endif //SERVER_H__

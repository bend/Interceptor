#ifndef SERVER_H__
#define SERVER_H__

#include <string>

#define SERVER_NAME		"Interceptor"
#define SERVER_VERSION	"0.1"

namespace Http {

  namespace Server {

    std::string getCommonName();
    std::string getVersion();
    std::string getName();
    std::string getOsName();

  }

}

#endif //SERVER_H__

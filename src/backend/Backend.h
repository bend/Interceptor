#ifndef BACKEND_H__
#define BACKEND_H__

#include <string>
#include <memory>

namespace Interceptor {
  namespace Backends {

    struct Backend {
      std::string name;
      std::string host;
      uint16_t port;
    };

    struct Connector : public Backend {
      enum Type {
        FCGI,
        Server
      };

      Type type;
    };

  }

}

#endif // BACKEND_H__

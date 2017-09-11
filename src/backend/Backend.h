#ifndef BACKEND_H__
#define BACKEND_H__

#include <string>
#include <memory>

namespace Interceptor {

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

  typedef std::shared_ptr<const Backend> BackendCPtr;
  typedef std::shared_ptr<Backend> BackendPtr;
  typedef std::shared_ptr<Connector> ConnectorPtr;

}

#endif // BACKEND_H__

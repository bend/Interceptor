#ifndef BACKEND_DEFS_H__
#define BACKEND_DEFS_H__

#include <memory>

namespace Interceptor {

  namespace Backends {
    struct Backend;
    class Connector;

  }

  typedef std::shared_ptr<const Backends::Backend> BackendCPtr;
  typedef std::shared_ptr<Backends::Backend> BackendPtr;
  typedef std::shared_ptr<Backends::Connector> ConnectorPtr;

}

#endif // BACKEND_DEFS_H__

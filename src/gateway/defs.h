#ifndef GATEWAY_DEFS_H__
#define GATEWAY_DEFS_H__

#include <memory>

namespace Interceptor {

  namespace Backends {

    class AbstractConnector;

  }
  typedef std::shared_ptr<Backends::AbstractConnector> AbstractConnectorPtr;
}


#endif // GATEWAY_DEFS_H__

#include "AbstractGateway.h"

namespace Interceptor::Backends {

  AbstractGateway::AbstractGateway(HttpRequestPtr request)
    : m_request(request)
  {

  }

}

#include "AbstractGateway.h"

#include "http/Request.h"

namespace Interceptor::Backends {

  AbstractGateway::AbstractGateway(HttpRequestPtr request)
    : m_request(request)
  {

  }

}

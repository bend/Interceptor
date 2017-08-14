#include "AbstractGateway.h"

namespace Interceptor {

  AbstractGateway::AbstractGateway(HttpRequestPtr request)
    : m_request(request)
  {

  }

}

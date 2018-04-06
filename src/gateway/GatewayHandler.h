#ifndef GATEWAY_HANDLER_H__
#define GATEWAY_HANDLER_H__

#include "http/Http.h"
#include "http/defs.h"
#include "core/Config.h"


#include <functional>

namespace Interceptor {

  class Params;

  namespace Backends {
    class AbstractGateway;
    class BackendsPool;


    class GatewayHandler {
    public:
      GatewayHandler(const std::string& backendName, HttpRequestPtr request,
                     BackendsPool* pool);
      ~GatewayHandler();

      void route(std::function<void(Http::StatusCode, std::stringstream*)> callback);

    protected:
      const SiteConfig*  m_site;
      HttpRequestPtr m_request;
      BackendsPool* m_pool;

      std::shared_ptr<AbstractGateway> m_gateway;

    };
  }

}

#endif // GatewayHandler

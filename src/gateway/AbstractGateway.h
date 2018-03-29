#ifndef ABSTRACT_GATEWAY_H__
#define ABSTRACT_GATEWAY_H__

#include "http/Request.h"
#include "AbstractConnector.h"

namespace Interceptor::Backends {

  class AbstractGateway {
  public:
    AbstractGateway(HttpRequestPtr request);

    virtual ~AbstractGateway() = default;

    virtual void setConnection(AbstractConnectorPtr connection) = 0;

    virtual AbstractConnectorPtr takeConnection() = 0;

    virtual void handleRequest(
      std::function<void(Http::StatusCode, std::stringstream*)>&
      func) = 0;
    virtual void reset() = 0;

  protected:
    HttpRequestPtr m_request;

  };

}

#endif // ABSTRACT_GATEWAY_H__

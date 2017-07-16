#ifndef ABSTRACT_GATEWAY_H__
#define ABSTRACT_GATEWAY_H__

#include "http/HttpRequest.h"
#include "AbstractConnector.h"

class AbstractGateway {
public:
  AbstractGateway(HttpRequestPtr request);

  virtual ~AbstractGateway() = default;

  virtual void setConnection(AbstractConnectorPtr connection) = 0;

  virtual AbstractConnectorPtr takeConnection() = 0;

  virtual void handleRequest(std::function<void(Http::Code, std::stringstream&)>
                             func) = 0;

protected:
  HttpRequestPtr m_request;

};

#endif // ABSTRACT_GATEWAY_H__

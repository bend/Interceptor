#ifndef BACKEND_GATEWAY_H__
#define BACKEND_GATEWAY_H__

#include "AbstractGateway.h"

class BackendGateway : public AbstractGateway {
public:
  BackendGateway(HttpRequestPtr request);

  virtual ~BackendGateway() = default;

  virtual void setConnection(AbstractConnectorPtr connection) override;

  virtual AbstractConnectorPtr takeConnection() override;

  virtual void handleRequest(std::function<void(Http::Code, std::stringstream&)>
                             func) override;

private:
  void forward(const char* data, size_t length,
               std::function<void(Http::Code, std::stringstream& )> callback);

private:
  AbstractConnectorPtr m_connection;
};

#endif // BACKEND_GATEWAY_H__

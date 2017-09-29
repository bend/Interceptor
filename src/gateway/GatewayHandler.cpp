#include "GatewayHandler.h"

#include "http/Request.h"
#include "utils/Logger.h"
#include "BackendGateway.h"
#include "common/Params.h"

namespace Interceptor {

  GatewayHandler::GatewayHandler(const std::string& backendName,
                                 HttpRequestPtr request,
                                 BackendsPool* pool)
    :
    m_request(request),
    m_pool(pool)
  {
    m_gateway = std::make_shared<BackendGateway>(m_request);
    AbstractConnectorPtr connector =
      m_pool->takeConnection(
        backendName);
    assert(connector);
    m_gateway->setConnection(connector);
  }

  GatewayHandler::~GatewayHandler()
  {
    LOG_DEBUG("GatewayHandler::~GatewayHandler()");
    m_gateway->reset();
    auto conn = m_gateway->takeConnection();
    conn->reset();
    conn.reset();
  }

  void GatewayHandler::route(
    std::function<void(Http::StatusCode, std::stringstream*)>
    func)
  {
    LOG_DEBUG("GatewayHandler::route()");
    m_gateway->handleRequest(func);
  }

}

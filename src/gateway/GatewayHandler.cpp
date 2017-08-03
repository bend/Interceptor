#include "GatewayHandler.h"

#include "http/HttpRequest.h"
#include "utils/Logger.h"
#include "BackendGateway.h"
#include "core/InterceptorSession.h"
#include "common/Params.h"

GatewayHandler::GatewayHandler(const SiteConfig* site, HttpRequestPtr request,
                               BackendsPool* pool)
  : m_site(site),
    m_request(request),
    m_pool(pool)
{
  if (m_site->m_backend.length() > 0) {
    m_gateway = std::make_shared<BackendGateway>(m_request);
    AbstractConnectorPtr connector =
      m_pool->takeConnection(
        m_site->m_backend);
    assert(connector);
    m_gateway->setConnection(connector);
  }
}

GatewayHandler::~GatewayHandler()
{
  LOG_DEBUG("GatewayHandler::~GatewayHandler()");
  m_gateway->reset();
  m_pool->putConnection(
    m_gateway->takeConnection());
}

void GatewayHandler::route(std::function<void(Http::Code, std::stringstream*)>
                           func)
{
  LOG_DEBUG("GatewayHandler::route()");
  m_gateway->handleRequest(func);
}

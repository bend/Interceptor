#include "GatewayHandler.h"

#include "http/HttpRequest.h"
#include "utils/Logger.h"
#include "BackendGateway.h"
#include "core/InterceptorSession.h"
#include "common/Params.h"

GatewayHandler::GatewayHandler(const SiteConfig* site, HttpRequestPtr request)
  : m_site(site),
    m_request(request)
{
  if (m_site->m_backend.length() > 0) {
    m_gateway = std::make_unique<BackendGateway>(m_request);
    AbstractConnectorPtr connector =
      m_request->session()->params()->backendsPool()->takeConnection(
        m_site->m_backend);
    assert(connector);
    m_gateway->setConnection(std::move(connector));
  }
}

GatewayHandler::~GatewayHandler()
{
  LOG_DEBUG("GatewayHandler::~GatewayHandler()");
  m_request->session()->params()->m_pool->putConnection(
    m_gateway->takeConnection());
}

void GatewayHandler::route(std::function<void(Http::Code, std::stringstream&)>
                           func)
{
  m_gateway->handleRequest(func);
}

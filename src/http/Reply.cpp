#include "Reply.h"

#include "Request.h"
#include "utils/Logger.h"
#include "cache/generic_cache.h"
#include "common/Params.h"
#include "common/Buffer.h"
#include "common/Redirection.h"
#include "gateway/GatewayHandler.h"
#include "core/SessionConnection.h"
#include "GetReply.h"
#include "HeadReply.h"
#include "ErrorReply.h"
#include "RedirectReply.h"
#include "HttpException.h"

#include "utils/StringUtils.h"

#include <algorithm>
#include <functional>

namespace Interceptor::Http {

  Reply::Reply(HttpRequestPtr request)
    : m_request(request),
      m_gateway(nullptr)
  {
  }

  Reply::~Reply()
  {
    LOG_DEBUG("Reply::~Reply()");
  }

  void Reply::process()
  {
    LOG_DEBUG("Reply::process()");

    try {
      std::stringstream stream;

      if ( !m_request->headersReceived() ) {
        std::stringstream stream;
        buildErrorResponse(Code::BadRequest, true);
        return;
      }

      m_request->parse();

      if (!m_request->hasMatchingSite()) {
        buildErrorResponse(Code::NotFound, true);
        return;
      }

      const SiteConfig* site = m_request->matchingSite();

      const auto redirection = site->redirection(m_request->host() +
                               m_request->index());

      if (redirection) {
        handleRedirection(redirection, site);
      } else if (hasGateway(site)) {
        handleGatewayRequest(site);
      } else {
        handleHttpMethod(site);

        auto buffer = m_reply->buildReply();
        post(buffer);
      }
    } catch (HttpException& e) {
      LOG_ERROR("HttpException: " << (int)e.code() );
      buildErrorResponse(e.code(), e.closeConnection());
    }
  }

  bool Reply::hasGateway(const SiteConfig* site) const
  {
    return gatewayName(site).length() > 0;
  }

  std::string Reply::gatewayName(const SiteConfig* site) const
  {
    try {
      if (site->m_backend.length() > 0) {
        return site->m_backend;
      }

      std::string path = CommonReply::requestedPath(m_request, site);

      return site->connectorName(path);
    } catch (HttpException& e) {
      return "";
    }
  }

  void Reply::handleHttpMethod(const SiteConfig* site)
  {
    LOG_DEBUG("Reply::handleHttpMethod()");

    switch (m_request->method()) {
      case Method::GET:
        m_reply = std::make_shared<GetReply>(m_request, site);
        break;

      case Method::HEAD:
        m_reply = std::make_shared<HeadReply>(m_request, site);
        break;

      case Method::POST:
        break;

      default:
        break;
    }
  }

  void Reply::handleRedirection(const Redirection* redirection,
                                const SiteConfig* site)
  {
    LOG_DEBUG("Reply::handleRedirection()");
    m_reply = std::make_shared<RedirectReply>(m_request, site, redirection);
    auto buffer = m_reply->buildReply();
    post(buffer);
  }

  void Reply::handleGatewayRequest(const SiteConfig* site)
  {
    LOG_DEBUG("Reply::handleGatewayRequest()");
    m_gateway = std::make_unique<GatewayHandler>(gatewayName(site), m_request,
                m_request->params()->m_pool);
    std::weak_ptr<Reply> wp {shared_from_this()};
    m_request->setCompleted(true);
    m_gateway->route(std::bind([wp] (Http::Code code, std::stringstream * stream) {
      auto ptr = wp.lock();

      if (ptr) {
        ptr->handleGatewayReply(code, stream);
      } else if (stream) {
        LOG_ERROR("Reply::process() : Reply is already destructed, cannot send");
      }
    }, std::placeholders::_1, std::placeholders::_2));
    return;

  }

  void Reply::handleGatewayReply(Code code, std::stringstream* stream)
  {
    LOG_DEBUG("Reply::handleGatewayReply()");

    if (code != Code::Ok || !stream) {
      buildErrorResponse(code, true);
    } else {
      LOG_NETWORK("Reply::handleGatewayReply() - got reply: ", stream->str());

      if (checkBackendReply(*stream)) {
        postBackendReply(*stream);
      } else {
        buildErrorResponse(Code::InternalServerError, true);
      }

      delete stream;
    }
  }

  void Reply::declineRequest(Code error)
  {
    LOG_DEBUG("Reply::declineRequest()");

    if (m_request->headersReceived())  {
      if (!m_request->parsed()) {
        try {
          m_request->parse();
        } catch (HttpException& e) {
          error = e.code();
        }
      }
    }

    buildErrorResponse(error, true);
  }

  void Reply::post(BufferPtr buffer)
  {
    auto connection = m_request->connection();

    if (connection) {
      connection->postReply(buffer);
    }
  }

  bool Reply::checkBackendReply(const std::stringstream& stream) const
  {
    //TODO
    return true;
  }

  void Reply::postBackendReply(const std::stringstream& stream)
  {
    LOG_DEBUG("Reply::postBackendReply()");
    auto buffer = std::make_shared<Buffer>();
    buffer->m_buffers.push_back(buffer->buf(std::string(stream.str())));

    auto connection = m_request->connection();

    if (connection) {
      connection->postReply(buffer);
    }
  }

  void Reply::buildErrorResponse(Code error, bool closeConnection)
  {
    LOG_DEBUG("Reply::buildErrorResponse()");
    auto site = m_request->matchingSite();
    m_reply = std::make_shared<ErrorReply>(m_request, site, error, closeConnection);
    auto buffer = m_reply->buildReply();
    post(buffer);
  }

}

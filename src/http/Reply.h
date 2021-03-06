#ifndef HTTP_REPLY_H__
#define HTTP_REPLY_H__

#include "defs.h"
#include "vars.h"
#include "Http.h"

#include "common/defs.h"
#include "core/Config.h"

#include <boost/asio.hpp>

namespace Interceptor {

  class Redirection;

  namespace Backends  {
    class GatewayHandler;
  }

  namespace Http {

    class CommonReply;

    enum class StatusCode : short;
      enum class Method : char;

      class Reply : public std::enable_shared_from_this<Reply> {

    public:

      Reply(HttpRequestPtr request);
      ~Reply();

      void process();

      void declineRequest(StatusCode error);

    private:
      void handleHttpMethod(const SiteConfig* site);
      void handleRedirection(const Redirection* r, const SiteConfig* site);
      void handleGatewayRequest(const SiteConfig* site);
      void handleModuleRequest(const SiteConfig* site);
      bool handleAuthentication(const SiteConfig* site);
      void handleGatewayReply(StatusCode code, std::stringstream*  stream);
      void handleModuleReply(BufferPtr buffer);

      bool checkBackendReply(const std::stringstream& stream) const;
      void post(BufferPtr buffer);
      void postBackendReply(const std::stringstream& stream);
      void buildErrorResponse(StatusCode error, bool closeConnection = false);
      bool hasAuthentication(const SiteConfig* config) const;
      bool hasGateway(const SiteConfig* config) const;
      bool hasModule(const SiteConfig* config) const;
      std::string gatewayName(const SiteConfig* config) const;
      std::string moduleName(const SiteConfig* config) const;
      std::string authName(const SiteConfig* config) const;

    private:
      typedef std::unique_ptr<Backends::GatewayHandler> GatewayHandlerUPtr;
      typedef std::shared_ptr<CommonReply> CommonReplyPtr;
      HttpRequestPtr m_request;
      GatewayHandlerUPtr m_gateway;
      CommonReplyPtr m_reply;
    };

  }

}
#endif // HTTP_REPLY_H__

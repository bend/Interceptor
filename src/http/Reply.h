#ifndef HTTP_REPLY_H__
#define HTTP_REPLY_H__

#include "common/Defs.h"
#include "vars.h"
#include "Http.h"
#include "core/Config.h"

#include <boost/asio.hpp>

namespace Interceptor {

  class GatewayHandler;

  namespace Http {

    class CommonReply;

    enum class Code : short;
    enum class Method : char;

    class Reply : public std::enable_shared_from_this<Reply> {

    public:

      Reply(HttpRequestPtr request);
      ~Reply();

      void process();

      void declineRequest(Code error);

    private:
      void handleHttpMethod(const SiteConfig* site);
      void handleGatewayRequest(const SiteConfig* site);
      void handleGatewayReply(Code code, std::stringstream*  stream);

      bool checkBackendReply(const std::stringstream& stream) const;
      void post(BufferPtr buffer);
      void postBackendReply(const std::stringstream& stream);
      void buildErrorResponse(Code error, bool closeConnection = false);
      bool hasGateway(const SiteConfig* config) const;
      std::string gatewayName(const SiteConfig* config) const;

    private:
      typedef std::unique_ptr<GatewayHandler> GatewayHandlerUPtr;
      typedef std::shared_ptr<CommonReply> CommonReplyPtr;
      HttpRequestPtr m_request;
      GatewayHandlerUPtr m_gateway;
      CommonReplyPtr m_reply;
    };

  }

}
#endif // HTTP_REPLY_H__

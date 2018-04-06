#include "ErrorReply.h"

#include "Request.h"
#include "utils/FileUtils.h"
#include "common/Buffer.h"

namespace Interceptor::Http {

  ErrorReply::ErrorReply(HttpRequestPtr request, const SiteConfig* config,
                         StatusCode error, bool closeConnection)
    : CommonReply(request, config)
  {
    m_status = error;
    setFlag(Flag::Closing, closeConnection);
  }

  BufferPtr ErrorReply::buildReply()
  {
    std::stringstream stream;
    bool found = false;

    m_replyHeaders->addGeneralHeaders();

    const ErrorPageMap& map = m_config ? m_config->m_errorPages :
                              m_request->params()->config()->m_errorPages;

    if (map.count(std::to_string((int)m_status)) > 0 ) {
      std::string url = map.at(std::to_string((int)m_status));

      if (m_request->cacheHandler()->read(url, stream,
                                          m_contentLength) == StatusCode::Ok) {
        found = true;
      }
    }

    std::string msg = formatMsg();

    if (!found) {
      stream << "<html>";
      stream << "	<body>";
      stream << "		<h1>";
      serializeHttpCode(m_status, stream);
      stream << "		</h1>";
      stream << "		<h3>";
      stream << msg;
      stream << "</h3>";
      stream << "	</body>";
      stream <<	"</html>";
      m_contentLength = stream.str().length();
    }

    buildHeaders(m_httpBuffer);

    m_httpBuffer->m_buffers.push_back(m_httpBuffer->buf(stream.str()));

    m_request->setCompleted(true);

    return m_httpBuffer;
  }

  std::string ErrorReply::formatMsg() const
  {
    switch (m_status) {
      case Http::StatusCode::BadRequest:
        return "The request is malformed";

      case Http::StatusCode::Forbidden:
        return "Access is Forbidden for the current URL";

      case Http::StatusCode::NotFound:
        return "The resource was not found on the server";

      case Http::StatusCode::RequestEntityTooLarge:
        return "The requested entity is too large";

      case Http::StatusCode::RequestRangeNotSatisfiable:
        return "The Requested Range is not possible for the URL";

      case Http::StatusCode::UnprocessableEntity:
        return "The request is not processable";

      case Http::StatusCode::InternalServerError:
        return "The server encountered an internal error";

      case Http::StatusCode::NotImplemented:
        return "The request is not supported by the server";

      case Http::StatusCode::ServiceUnavailable:
        return "The service is unavailable";

      case Http::StatusCode::HttpVersionNotSupported:
        return "The HTTP version is not supported by the server";

      default:
        return "";
    }
  }


}

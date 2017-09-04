#include "ErrorReply.h"

#include "utils/FileUtils.h"
#include "common/Buffer.h"

namespace Interceptor::Http {

  ErrorReply::ErrorReply(HttpRequestPtr request, const SiteConfig* config,
                         Code error, bool closeConnection)
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

      if (m_request->cacheHandler()->read(url, stream, m_contentLength) == Code::Ok) {
        found = true;
      }
    }

    if (!found) {
      stream << "<html> <body><h1>";
      stringValue(m_status, stream);
      stream << " </h1></body></html>";
      m_contentLength = stream.str().length();
    }

    buildHeaders(m_httpBuffer);

    m_httpBuffer->m_buffers.push_back(m_httpBuffer->buf(stream.str()));

    m_request->setCompleted(true);

    return m_httpBuffer;
  }

}

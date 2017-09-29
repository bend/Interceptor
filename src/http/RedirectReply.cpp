#include "RedirectReply.h"

#include "common/Buffer.h"

namespace Interceptor::Http {

  RedirectReply::RedirectReply(HttpRequestPtr request, const SiteConfig* site,
                               const Redirection* redirection)
    : CommonReply(request, site),
      m_redirection(redirection)
  {
    setFlag(Flag::Closing, true);
  }

  BufferPtr RedirectReply::buildReply()
  {
    std::stringstream stream;
    m_status = m_redirection->type() == Redirection::Permanent ?
               StatusCode::MovedPermanently : StatusCode::Found;

    m_replyHeaders->addHeader("Location", m_redirection->redirectURL());

    buildHeaders(m_httpBuffer);

    m_httpBuffer->m_buffers.push_back(m_httpBuffer->buf(stream.str()));

    m_request->setCompleted(true);

    return m_httpBuffer;
  }

}

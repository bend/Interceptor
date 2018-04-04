#include "AuthenticationReply.h"

#include "common/Buffer.h"
#include "authentication/Authentication.h"

namespace Interceptor::Http {

  AuthenticationReply::AuthenticationReply(HttpRequestPtr request, const SiteConfig* site)
    : CommonReply(request, site)
  {
    setFlag(Flag::Closing, true);
  }

  BufferPtr AuthenticationReply::buildReply()
  {
    std::stringstream stream;

	const std::string authName = m_config->authenticatorName(m_request->index());

	auto auth = m_request->params()->m_authLoader->get(authName);

	std::string authType = "Basic";
	std::string realm = m_config->realm(m_request->index());

    m_replyHeaders->addHeader("WWW-Authenticate", authType + " realm=\"" + realm + "\", charset=\"UTF-8\"");
	m_status = StatusCode::Unauthorized;

    buildHeaders(m_httpBuffer);

    m_httpBuffer->m_buffers.push_back(m_httpBuffer->buf(stream.str()));

    m_request->setCompleted(true);

    return m_httpBuffer;
  }

}

#include "Http2UpgradeReply.h"

#include "Request.h"
#include "utils/FileUtils.h"
#include "common/Buffer.h"

namespace Interceptor::Http {

  Http2UpgradeReply::Http2UpgradeReply(HttpRequestPtr request)
    : CommonReply(request, nullptr)
  {
    m_status = StatusCode::SwitchingProtocol;
    setFlag(Flag::Closing, false);
  }

  BufferPtr Http2UpgradeReply::buildReply()
  {
    std::stringstream stream;

    m_replyHeaders->addGeneralHeaders();

    m_replyHeaders->addHeader("Connection", "Upgrade");
    m_replyHeaders->addHeader("Upgrade", "h2c");

    buildHeaders(m_httpBuffer);

    m_httpBuffer->m_buffers.push_back(m_httpBuffer->buf(stream.str()));

    m_request->setCompleted(true);

    return m_httpBuffer;
  }


}

#include "DirectoryListingReply.h"

#include "Request.h"
#include "HttpException.h"
#include "utils/FileUtils.h"
#include "common/Buffer.h"

namespace Interceptor::Http {

  DirectoryListingReply::DirectoryListingReply(HttpRequestPtr request,
      const SiteConfig* config, const std::string& path)
    : CommonReply(request, config),
      m_path(path)
  {
    setFlag(Flag::Closing, false);
  }

  BufferPtr DirectoryListingReply::buildReply()
  {
    std::stringstream stream;

    if (m_config->listingAllowed(m_path)) {

      stream << "Index of " << m_request->index();
      std::vector<std::string> contents = FileUtils::directoryContents(m_path);

      for (auto& d : contents) {
        stream << d << "\n";
      }

      setHeadersFor(m_path);
      m_contentLength = stream.str().size();
    } else {
      throw HttpException(StatusCode::Forbidden, false);
    }

    m_replyHeaders->addGeneralHeaders();

    buildHeaders(m_httpBuffer);

    m_httpBuffer->m_buffers.push_back(m_httpBuffer->buf(stream.str()));

    m_request->setCompleted(true);

    return m_httpBuffer;
  }

  size_t DirectoryListingReply::size()
  {
    return m_contentLength;
  }



}

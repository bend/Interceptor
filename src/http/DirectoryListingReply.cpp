#include "DirectoryListingReply.h"

#include "Request.h"
#include "HttpException.h"
#include "utils/FileUtils.h"
#include "common/Buffer.h"


namespace Interceptor::Http {

  namespace Html {
    extern  std::vector<const char*> HtmlHeader();
    extern  std::vector<const char*> HtmlFooter();
  }

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

      std::vector<std::string> contents = FileUtils::directoryContents(m_path);

      auto header = Html::HtmlHeader();

      for (auto& data : header) {
        stream << data;
      }

      for (auto& d : contents) {
        stream << "<a href=\"" << m_request->index() << "/" << d << "\">" << d <<
               "</a>\n";
      }

      auto footer = Html::HtmlFooter();

      for (auto& data : footer) {
        stream << data;
      }

      m_contentLength = stream.str().size();
      setHeadersFor(m_path);
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

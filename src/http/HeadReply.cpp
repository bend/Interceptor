#include "HeadReply.h"

#include "Request.h"
#include "HttpException.h"
#include "DirectoryListingReply.h"
#include "utils/FileUtils.h"
#include "common/Buffer.h"


namespace Interceptor::Http {

  HeadReply::HeadReply(HttpRequestPtr request, const SiteConfig* config)
    : CommonReply(request, config)
  {}

  BufferPtr HeadReply::buildReply()
  {
    LOG_DEBUG("HeadReply::handleRetrievalRequest()");

    StatusCode ret;

    if ( (ret = getLocationCode(m_config)) != StatusCode::Ok) {
      throw HttpException(ret, true);
    }

    requestFileHeader();

    return m_httpBuffer;
  }

  void HeadReply::requestFileHeader()
  {
    LOG_DEBUG("HeadReply::requestFileContents()");
    std::string page;

    page = requestedPath();

    setHeadersFor(page);

    if (FileUtils::isDirectory(page)) {
      requestDirectorySize(page);
    } else if (!m_request->cacheHandler()->size(page, m_contentLength)) {
      throw HttpException(StatusCode::NotFound, false);
    }

    buildHeaders(m_httpBuffer);
  }

  void HeadReply::requestDirectorySize(const std::string& path)
  {
    auto ptr = std::make_shared<DirectoryListingReply>(m_request, m_config, path);
    ptr->buildReply();
    m_contentLength = ptr->size();
  }

}

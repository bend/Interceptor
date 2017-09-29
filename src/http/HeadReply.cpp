#include "HeadReply.h"

#include "HttpException.h"
#include "utils/FileUtils.h"


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

    if (!m_request->cacheHandler()->size(page, m_contentLength)) {
      throw HttpException(StatusCode::NotFound, false);
    }

    buildHeaders(m_httpBuffer);
  }

}

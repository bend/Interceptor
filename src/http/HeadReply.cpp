#include "HeadReply.h"

#include "Request.h"
#include "HttpException.h"
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

    if (FileUtils::isDirectory(page)) {
      //TODO
      return;
    }

    setHeadersFor(page);

    if (!m_request->cacheHandler()->size(page, m_contentLength)) {
      throw HttpException(StatusCode::NotFound, false);
    }

    buildHeaders(m_httpBuffer);
  }

}

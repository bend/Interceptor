#include "PostReply.h"

#include "utils/Logger.h"

namespace Interceptor::Http {

  PostReply::PostReply(HttpRequestPtr request, const SiteConfig* site)
    : GetReply(request, site)
  {
  }

  void PostReply::processRequest()
  {
    LOG_DEBUG("PostRequest::processRequest()");
    //TODO check body in Request
    // We also need to implement chunking probably to
    // parse the request.
    requestFileContents();
  }

}

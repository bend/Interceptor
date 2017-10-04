#ifndef POST_REPLY_H__
#define POST_REPLY_H__

#include "GetReply.h"

namespace Interceptor::Http {

  class PostReply : public GetReply {
  public:
    PostReply(HttpRequestPtr request, const SiteConfig* site);

  private:
    virtual void processRequest() override;

  };

}

#endif





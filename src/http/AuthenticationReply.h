#ifndef AUTHENTICATION_REPLY_H__
#define AUTHENTICATION_REPLY_H__

#include "CommonReply.h"

namespace Interceptor::Http {

  class AuthenticationReply : public CommonReply {
  public:
    AuthenticationReply(HttpRequestPtr request, const SiteConfig* site);
    virtual BufferPtr buildReply() override;

  protected:
    virtual void serialize(std::stringstream& stream) override {};

  };


}

#endif // AUTHENTICATION_REPLY_H__

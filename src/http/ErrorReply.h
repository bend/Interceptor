#ifndef ERROR_REPLY_H__
#define ERROR_REPLY_H__

#include "CommonReply.h"

namespace Interceptor::Http {

  class ErrorReply : public CommonReply {
  public:
    ErrorReply(HttpRequestPtr request, const SiteConfig* site, Code errorCode,
               bool closeConnection);

    virtual BufferPtr buildReply() override;

  protected:
    virtual void serialize(std::stringstream& stream) override {};

  };

}

#endif // ERROR_REPLY_H__

#ifndef REDIRECT_REPLY_H__
#define REDIRECT_REPLY_H__

#include "CommonReply.h"

namespace Interceptor::Http {

  class RedirectReply : public CommonReply {
  public:
    RedirectReply(HttpRequestPtr request, const SiteConfig* site,
                  const Redirection* redirection);
    virtual BufferPtr buildReply() override;

  protected:
    virtual void serialize(std::stringstream& stream) override {};

  private:
    const Redirection* m_redirection;
  };


}

#endif // REDIRECT_REPLY_H__

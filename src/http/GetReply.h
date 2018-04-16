#ifndef GET_REPLY_H__
#define GET_REPLY_H__

#include "CommonReply.h"

namespace Interceptor::Http {

  class GetReply : public CommonReply {
  public:
    GetReply(HttpRequestPtr request, const SiteConfig* site);

    virtual BufferPtr buildReply() override;

  protected:
    virtual void serialize(std::stringstream& stream) override;
    virtual void processRequest();
    void requestFileContents();
    void requestDirectory(const std::string& path, std::stringstream& stream,
                          size_t& bytes);
    bool requestIfMofidiedSince();
  };

}

#endif // GET_REPLY_H__

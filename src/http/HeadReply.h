#ifndef HEAD_REPLY_H__
#define HEAD_REPLY_H__

#include "CommonReply.h"

#include "common/defs.h"

namespace Interceptor::Http {

  class HeadReply : public CommonReply {
  public:
    HeadReply(HttpRequestPtr request, const SiteConfig* config);

    virtual BufferPtr buildReply() override;

  protected:
    virtual void serialize(std::stringstream& stream) override {};

  private:
    void requestFileHeader();
    void requestDirectorySize(const std::string& path);

  };

}

#endif

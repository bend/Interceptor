#ifndef HTTP2_UPGRADE_REPLY_H__
#define HTTP2_UPGRADE_REPLY_H__

#include "CommonReply.h"

namespace Interceptor::Http {

  class Http2UpgradeReply : public CommonReply {
  public:
    Http2UpgradeReply(HttpRequestPtr request);

    virtual BufferPtr buildReply() override;

  protected:
    virtual void serialize(std::stringstream& stream) override {};
    void serializeDetails(StatusCode status, std::stringstream& stream);
    std::string formatMsg() const;

  };

}

#endif // HTTP2_UPGRADE_REPLY_H__

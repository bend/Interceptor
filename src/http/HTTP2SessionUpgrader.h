#ifndef HTTP2_SESSION_UPGRADER_H__
#define HTTP2_SESSION_UPGRADER_H__

#include "defs.h"

#include "common/AbstractSessionHandler.h"
#include "core/defs.h"

#include <boost/system/error_code.hpp>

namespace Interceptor::Http {


  class HTTP2SessionUpgrader : public AbstractSessionHandler,
    public std::enable_shared_from_this<HTTP2SessionUpgrader> {

  public:
    HTTP2SessionUpgrader(SessionConnectionPtr connection);
    ~HTTP2SessionUpgrader();

    virtual void transferSession(const char* data, size_t bytes) override;

  private:
    void processData(const char* data, size_t bytes);
    void read();
    void handleHttpRequestRead(const boost::system::error_code& error,
                               size_t bytesTransferred);
    void send101Continue();

  private:

    char m_requestBuffer[4096];
    HttpRequestPtr m_request;
    CommonReplyPtr m_reply;

  };

}

#endif // HTTP2_SESSION_UPGRADER_H__

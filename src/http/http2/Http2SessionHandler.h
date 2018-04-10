#ifndef HTTP2_SESSION_HANDLER_H__
#define HTTP2_SESSION_HANDLER_H__

#include "defs.h"

#include "common/AbstractSessionHandler.h"

#include "core/defs.h"

#include <boost/system/error_code.hpp>

namespace Interceptor::Http {


  class Http2SessionHandler : public AbstractSessionHandler,
    public std::enable_shared_from_this<Http2SessionHandler> {

  public:
    Http2SessionHandler(SessionConnectionPtr connection);
    ~Http2SessionHandler();

    virtual void transferSession(const char* data, size_t bytes) override;

  private:
    void processData(const char* data, size_t bytes);
    void read();
    void handleHttpRequestRead(const boost::system::error_code& error,
                               size_t bytesTransferred);

  private:

    char m_requestBuffer[4096];
    HttpRequestPtr m_request;
    HttpReplyPtr m_reply;


  };

}

#endif // HTTP2_SESSION_HANDLER_H__

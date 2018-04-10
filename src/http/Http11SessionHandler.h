#ifndef HTTP11_SESSION_HANDLER_H__
#define HTTP11SessionHandler_H__

#include "defs.h"

#include "common/AbstractSessionHandler.h"

#include "core/defs.h"

#include <boost/system/error_code.hpp>

namespace Interceptor::Http {


  class Http11SessionHandler : public AbstractSessionHandler,
    public std::enable_shared_from_this<Http11SessionHandler> {

  public:
    Http11SessionHandler(SessionConnectionPtr connection);
    ~Http11SessionHandler();

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
    HttpReplyPtr m_reply;


  };

}

#endif // HTTP11_SESSION_HANDLER_H__

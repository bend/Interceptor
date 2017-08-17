#ifndef HTTP1X_SESSION_HANDLER_H__
#define HTTP1XSessionHandler_H__

#include "common/AbstractSessionHandler.h"

#include "common/Defs.h"

#include <boost/system/error_code.hpp>

namespace Interceptor::Http {


  class HTTP1XSessionHandler : public AbstractSessionHandler,
    public std::enable_shared_from_this<HTTP1XSessionHandler> {

  public:
    HTTP1XSessionHandler(SessionConnectionPtr connection);
    ~HTTP1XSessionHandler();

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

#endif // HTTP1X_SESSION_HANDLER_H__

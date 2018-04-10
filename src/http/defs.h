#ifndef HTTP_DEFS_H__
#define HTTP_DEFS_H__

#include <memory>

namespace Interceptor {
  namespace Http {
    class Request;
    class Reply;
    class Headers;
    class CommonReply;
  }

  typedef std::shared_ptr<Http::Request> HttpRequestPtr;
  typedef std::shared_ptr<Http::Reply> HttpReplyPtr;
  typedef std::unique_ptr<Http::Headers> HttpHeaderUPtr;
  typedef std::shared_ptr<Http::CommonReply> CommonReplyPtr;

}

#endif // HTTP_FWD_H__

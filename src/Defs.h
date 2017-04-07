#ifndef DEFS_H__

#include <memory>

#define MAX_CHUNK_SIZE 200*1024

class InterceptorSession;
class InboundConnection;

namespace Http {
  class HttpRequest;
  class HttpReply;

};

typedef std::shared_ptr<InterceptorSession> InterceptorSessionPtr;
typedef std::shared_ptr<InboundConnection> InboundConnectionPtr;
typedef std::shared_ptr<Http::HttpRequest> HttpRequestPtr;
typedef std::shared_ptr<Http::HttpReply> HttpReplyPtr;

typedef std::string Host;

#endif //DEFS_H__

#ifndef DEFS_H__

#include <memory>

#define MAX_CHUNK_SIZE 1024 * 1024

class InterceptorSession;
class InboundConnection;

namespace Http {
  class HttpRequest;
  class HttpReply;
  class HttpBuffer;
};

typedef std::shared_ptr<InterceptorSession> InterceptorSessionPtr;
typedef std::weak_ptr<InterceptorSession> InterceptorSessionWeakPtr;
typedef std::shared_ptr<InboundConnection> InboundConnectionPtr;
typedef std::shared_ptr<Http::HttpRequest> HttpRequestPtr;
typedef std::shared_ptr<Http::HttpReply> HttpReplyPtr;
typedef std::shared_ptr<Http::HttpBuffer> HttpBufferPtr;
typedef std::pair<const char*, size_t> Packet;

typedef std::string Host;

#endif //DEFS_H__

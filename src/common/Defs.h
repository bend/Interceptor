#ifndef DEFS_H__

#include <memory>

#define MAX_CHUNK_SIZE 1024 * 1024

namespace Interceptor {

  class Session;
  class InboundConnection;
  class SessionConnection;
  class Params;
  class Buffer;

  namespace Http {
    class Request;
    class Reply;
    class Headers;
  }

  typedef std::shared_ptr<Session> SessionPtr;
  typedef std::weak_ptr<Session> SessionWeakPtr;
  typedef std::shared_ptr<InboundConnection> InboundConnectionPtr;
  typedef std::shared_ptr<SessionConnection> SessionConnectionPtr;
  typedef std::shared_ptr<Http::Request> HttpRequestPtr;
  typedef std::shared_ptr<Http::Reply> HttpReplyPtr;
  typedef std::shared_ptr<Buffer> BufferPtr;
  typedef std::shared_ptr<Params> ParamsPtr;
  typedef std::unique_ptr<Http::Headers> HttpHeaderUPtr;

  typedef std::string Host;

}

#endif //DEFS_H__

#ifndef DEFS_H__

#include <memory>

class InterceptorSession;
class InboundConnection;
class HttpRequest;
class HttpReply;

typedef std::shared_ptr<InterceptorSession> InterceptorSessionPtr;
typedef std::shared_ptr<InboundConnection> InboundConnectionPtr;
typedef std::shared_ptr<HttpRequest> HttpRequestPtr;
typedef std::shared_ptr<HttpReply> HttpReplyPtr;

typedef std::string Host;

#endif //DEFS_H__

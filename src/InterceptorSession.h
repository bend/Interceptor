#ifndef INTERCEPTOR_SESSION_H__
#define INTERCEPTOR_SESSION_H__

#include "Defs.h"

#include <boost/asio.hpp>

class InboundConnection;

class InterceptorSession : public std::enable_shared_from_this<InterceptorSession>  {

public:
  InterceptorSession(boost::asio::io_service& ioService);
  ~InterceptorSession() = default;

  boost::asio::ip::tcp::socket& socket() const;

  InboundConnectionPtr connection() const;

  void start();

private:
  void handleHttpRequestRead(const boost::system::error_code& error, size_t bytesTransferred);
  void parseHttpRequest(const std::string& request);


private:
  boost::asio::io_service& m_ioService;
  InboundConnectionPtr m_connection;
  unsigned char m_requestBuffer[4096];
  HttpRequestPtr m_request;
  HttpReplyPtr m_reply;
};

#endif //INTERCEPTOR_SESSION_H__

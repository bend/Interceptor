#ifndef INTERCEPTOR_SESSION_H__
#define INTERCEPTOR_SESSION_H__

#include "Defs.h"

#include "Config.h"

#include <boost/asio.hpp>
#include <deque>

class InboundConnection;

namespace Http {
  class HttpBuffer;
}

class InterceptorSession : public
  std::enable_shared_from_this<InterceptorSession>  {

public:

  InterceptorSession(const Config::ServerConfig* config,
                     boost::asio::io_service& ioService);
  ~InterceptorSession() = default;

  boost::asio::ip::tcp::socket& socket() const;

  InboundConnectionPtr connection() const;

  const Config::ServerConfig* config() const;

  void start();

  void postReply(Http::HttpBuffer* httpBuffer);

  void closeConnection();


private:
  // handlers
  void handleHttpRequestRead(const boost::system::error_code& error,
                             size_t bytesTransferred);
  void handleTransmissionCompleted(Http::HttpBuffer* httpBuffer,
                                   const boost::system::error_code& error, size_t bytesTransferred);

  // internal logic
  void sendReply(Http::HttpBuffer* buffer);
  void sendNext(Http::HttpBuffer* buffer);

  void startReadTimer();
  void startWriteTimer();
  void stopReadTimer();
  void stopWriteTimer();
  void handleTimeout(const boost::system::error_code& error);

private:
  const Config::ServerConfig* m_config;
  boost::asio::io_service& m_ioService;
  boost::asio::strand m_strand;
  InboundConnectionPtr m_connection;
  unsigned char m_requestBuffer[4096];
  HttpRequestPtr m_request;
  HttpReplyPtr m_reply;

  std::deque<Http::HttpBuffer*> m_buffers;

  // Timers
  boost::asio::deadline_timer m_readTimer;
  boost::asio::deadline_timer m_writeTimer;

  bool m_canSend;
};

#endif //INTERCEPTOR_SESSION_H__

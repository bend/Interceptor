#ifndef INTERCEPTOR_SESSION_H__
#define INTERCEPTOR_SESSION_H__

#include "Defs.h"

#include "Config.h"

#include <boost/asio.hpp>
#include <deque>

class InboundConnection;

class InterceptorSession : public std::enable_shared_from_this<InterceptorSession>  {

public:

  InterceptorSession(const Config::ServerConfig* config, boost::asio::io_service& ioService);
  ~InterceptorSession() = default;

  boost::asio::ip::tcp::socket& socket() const;

  InboundConnectionPtr connection() const;

  const Config::ServerConfig* config() const;

  void start();

  void postReply(HttpReplyPtr reply);

  void closeConnection();


private:
  // handlers
  void handleHttpRequestRead(const boost::system::error_code& error, size_t bytesTransferred);
  void handleTransmissionCompleted(HttpReplyPtr reply, const boost::system::error_code& error, size_t bytesTransferred);

  // internal logic
  void sendReply(HttpReplyPtr reply);

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

  // Timers
  boost::asio::deadline_timer m_readTimer;
  boost::asio::deadline_timer m_writeTimer;
};

#endif //INTERCEPTOR_SESSION_H__

#ifndef INTERCEPTOR_SESSION_H__
#define INTERCEPTOR_SESSION_H__

#include "Defs.h"

#include "Config.h"

#include <boost/asio.hpp>
#include <deque>
#include <boost/thread/mutex.hpp>

class InboundConnection;
class AbstractCacheHandler;

class InterceptorSession : public
  std::enable_shared_from_this<InterceptorSession>  {

public:

  InterceptorSession(const Config::ServerConfig* config,
                     AbstractCacheHandler* cache,
                     boost::asio::io_service& ioService);

  ~InterceptorSession();

  boost::asio::ip::tcp::socket& socket() const;

  InboundConnectionPtr connection() const;

  const Config::ServerConfig* config() const;

  void start();

  void postReply(HttpBufferPtr httpBuffer);

  void closeConnection();

private:
  enum TimerType : uint8_t {
    ReadTimer = 0x01,
    WriteTimer = 0x02
  };
  enum State : uint8_t  {
    CanSend = 0x01,
    Reading = 0x02,
    Sending = 0x04,
	Closing = 0x08
  };

private:
  // handlers
  void handleHttpRequestRead(const boost::system::error_code& error,
                             size_t bytesTransferred);
  void handleTransmissionCompleted(HttpBufferPtr httpBuffer,
                                   const boost::system::error_code& error, size_t bytesTransferred);

  // internal logic
  void sendReply(HttpBufferPtr buffer);
  void sendNext(HttpBufferPtr buffer);

  void startReadTimer();
  void startWriteTimer();
  void stopReadTimer();
  void stopWriteTimer();
  void handleTimeout(TimerType timerType, const boost::system::error_code& error);
  void doRead();
  void doCloseConnection();

private:
  const Config::ServerConfig* m_config;
  AbstractCacheHandler* m_cache;
  boost::asio::io_service& m_ioService;
  boost::asio::strand m_iostrand;
  boost::asio::strand m_fsstrand;
  InboundConnectionPtr m_connection;
  unsigned char m_requestBuffer[4096];
  HttpRequestPtr m_request;
  HttpReplyPtr m_reply;

  std::deque<HttpBufferPtr> m_buffers;
  boost::mutex m_buffersMutex;

  // Timers
  boost::asio::deadline_timer m_readTimer;
  boost::asio::deadline_timer m_writeTimer;

  int m_state;


};

#endif //INTERCEPTOR_SESSION_H__

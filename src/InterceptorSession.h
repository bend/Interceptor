#ifndef INTERCEPTOR_SESSION_H__
#define INTERCEPTOR_SESSION_H__

#include "Defs.h"

#include <boost/asio.hpp>
#include <deque>

class InboundConnection;

class InterceptorSession : public std::enable_shared_from_this<InterceptorSession>  {

public:

  struct Packet {
    Packet(unsigned char* data, size_t size);
    Packet() = default;
    ~Packet();
    unsigned char* m_data;
    size_t m_size;
  };

  InterceptorSession(boost::asio::io_service& ioService);
  ~InterceptorSession() = default;

  boost::asio::ip::tcp::socket& socket() const;

  InboundConnectionPtr connection() const;

  void start();

  void postResponse(Packet* packet);

private:
  // handlers
  void handleHttpRequestRead(const boost::system::error_code& error, size_t bytesTransferred);
  void handleTransmissionCompleted(Packet* packet, const boost::system::error_code& error, size_t bytesTransferred);

  // internal logic
  void sendResponse(Packet* packet);


private:
  boost::asio::io_service& m_ioService;
  boost::asio::strand m_strand;
  InboundConnectionPtr m_connection;
  unsigned char m_requestBuffer[4096];
  HttpRequestPtr m_request;
  HttpReplyPtr m_reply;
  std::deque<Packet*> m_outbox;
};

#endif //INTERCEPTOR_SESSION_H__

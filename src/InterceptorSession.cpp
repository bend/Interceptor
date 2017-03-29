#include "InterceptorSession.h"

#include "Defs.h"
#include "Logger.h"

#include "socket/InboundConnection.h"
#include "http/HttpRequest.h"
#include "http/HttpReply.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

InterceptorSession::Packet::Packet(unsigned char* data, size_t size)
  : m_data(data),
    m_size(size)
{
}

InterceptorSession::Packet::~Packet()
{
  delete[] m_data;
}

InterceptorSession::InterceptorSession(const Config::ServerConfig* config, boost::asio::io_service& ioService)
  : m_config(config),
    m_ioService(ioService),
    m_strand(ioService)
{
  m_connection = std::make_shared<TcpInboundConnection>(m_ioService);
}

boost::asio::ip::tcp::socket& InterceptorSession::socket() const
{
  return m_connection->socket();
}

InboundConnectionPtr InterceptorSession::connection() const
{
  return m_connection;
}

const Config::ServerConfig* InterceptorSession::config() const
{
  return m_config;
}

void InterceptorSession::postReply(HttpReplyPtr reply)
{
  m_ioService.post(
    m_strand.wrap(
      boost::bind(&InterceptorSession::sendReply, shared_from_this(), reply)));
}


void InterceptorSession::sendReply(HttpReplyPtr reply)
{
  /*
  m_connection->asyncWrite(packet->m_data, packet->m_size, m_strand.wrap
  (boost::bind
    (&InterceptorSession::handleTransmissionCompleted,
     shared_from_this(),
     packet,
     boost::asio::placeholders::error,
     boost::asio::placeholders::bytes_transferred)));
     */
}

void InterceptorSession::handleTransmissionCompleted(HttpReplyPtr reply, const boost::system::error_code& error, size_t bytesTransferred)
{
  if (!error)  {
    trace("debug") << "Response sent " ;
  } else {
    trace("error") << "Could not send reponse " << error.message();
    //TODO handle
  }
}

void InterceptorSession::closeConnection()
{
  m_ioService.post(
    m_strand.wrap(
      boost::bind(&InboundConnection::disconnect, m_connection)));
}

void InterceptorSession::start()
{
  // Avoid Slow Loris attacks, close connection after 2min if
  // no preamble received
  // Read preamble Size from mobile
  InterceptorSessionPtr isp = shared_from_this();
  if (m_connection) {
    m_connection->asyncReadSome(m_requestBuffer, sizeof(m_requestBuffer),
                                boost::bind(&InterceptorSession::handleHttpRequestRead, isp,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred)
                               );
  }
}

void InterceptorSession::handleHttpRequestRead(const boost::system::error_code& error, size_t bytesTransferred)
{
  if (!error) {
    trace("info") << "Request read from " << m_connection->ip();
    if (!m_request || m_request->completed() ) {
      // Create Request
      m_request = std::make_shared<HttpRequest>(shared_from_this());
    }
    m_request->appendData(m_requestBuffer, bytesTransferred);
    if (!m_request->headersReceived()) {
      start();
    } else  {
      // complete headers received
      m_reply = std::make_shared<HttpReply>(m_request);
      m_reply->process();
      start();
    }
  } else {
    trace("error") << "Error reading request from " << m_connection->ip();
  }
}


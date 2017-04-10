#include "InterceptorSession.h"

#include "Defs.h"
#include "utils/Logger.h"

#include "socket/InboundConnection.h"
#include "http/HttpRequest.h"
#include "http/HttpBuffer.h"
#include "http/HttpReply.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

InterceptorSession::InterceptorSession(const Config::ServerConfig* config,
                                       boost::asio::io_service& ioService)
  : m_config(config),
    m_ioService(ioService),
    m_strand(ioService),
    m_readTimer(ioService),
    m_writeTimer(ioService),
    m_canSend(true)
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

void InterceptorSession::postReply(Http::HttpBuffer* buffer)
{
  m_ioService.post(
    m_strand.wrap(
      boost::bind(&InterceptorSession::sendNext, shared_from_this(), buffer)));
}

void InterceptorSession::sendNext(Http::HttpBuffer* buffer)
{
  m_buffers.push_back(buffer);

  if (m_canSend) {
    auto v = m_buffers.front();
    m_buffers.pop_front();
    sendReply(v);
  }
}

void InterceptorSession::sendReply(Http::HttpBuffer* buffer)
{
  startWriteTimer();
  m_canSend = false;
  m_connection->asyncWrite(buffer->m_buffers, m_strand.wrap
                           (boost::bind
                            (&InterceptorSession::handleTransmissionCompleted,
                             shared_from_this(),
                             buffer,
                             boost::asio::placeholders::error,
                             boost::asio::placeholders::bytes_transferred)
                           )
                          );
}

void InterceptorSession::handleTransmissionCompleted(
  Http::HttpBuffer* buffer,
  const boost::system::error_code& error, size_t bytesTransferred)
{
  stopWriteTimer();

  if (!error)  {
    LOG_DEBUG("Response sent ");
    m_canSend = true;


    if (!m_buffers.empty()) {
      auto v = m_buffers.front();
      m_buffers.pop_front();
      sendReply(v);
    }

  } else {
    LOG_ERROR("Could not send reponse " << error.message());
    closeConnection();
  }

  //TODO free memory

  /* XXX
  if (reply->getFlag(Http::HttpReply::Closing)) {
    closeConnection();
  }
  */
}

void InterceptorSession::closeConnection()
{
  m_ioService.post(
    m_strand.wrap(
      boost::bind(&InboundConnection::disconnect, m_connection)));
}

void InterceptorSession::start()
{
  // Avoid Slow Loris attacks, close connection if nothing read
  InterceptorSessionPtr isp = shared_from_this();

  if (m_connection) {
    startReadTimer();
    m_connection->asyncReadSome(m_requestBuffer, sizeof(m_requestBuffer),
                                boost::bind(&InterceptorSession::handleHttpRequestRead, isp,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred)
                               );
  }
}

void InterceptorSession::handleHttpRequestRead(const boost::system::error_code&
    error, size_t bytesTransferred)
{
  stopReadTimer();

  if (!error) {
    LOG_INFO("Request read from " << m_connection->ip());

    if (!m_request || m_request->completed() ) {
      // Create Request
      m_request = std::make_shared<Http::HttpRequest>(shared_from_this());
    }

    m_request->appendData(m_requestBuffer, bytesTransferred);

    if (!m_request->headersReceived()) {
      start();
    } else  {
      // complete headers received
      m_reply = std::make_shared<Http::HttpReply>(m_request);
      m_reply->process();
      start();
    }
  } else {
    LOG_ERROR("Error reading request from " << m_connection->ip());
  }
}

void InterceptorSession::startReadTimer()
{
  LOG_DEBUG("Setting timeout to " << m_config->m_clientTimeout);
  m_readTimer.expires_from_now(boost::posix_time::seconds(
                                 m_config->m_clientTimeout));
  m_readTimer.async_wait
  (m_strand.wrap
   (boost::bind(&InterceptorSession::handleTimeout,
                shared_from_this(),
                boost::asio::placeholders::error)));

}

void InterceptorSession::startWriteTimer()
{
  m_writeTimer.expires_from_now(boost::posix_time::seconds(
                                  m_config->m_serverTimeout));
  m_writeTimer.async_wait
  (m_strand.wrap
   (boost::bind(&InterceptorSession::handleTimeout,
                shared_from_this(),
                boost::asio::placeholders::error)));
}

void InterceptorSession::stopReadTimer()
{
  LOG_DEBUG("cancel read timer");
  m_readTimer.cancel();
}

void InterceptorSession::stopWriteTimer()
{
  m_writeTimer.cancel();
}

void InterceptorSession::handleTimeout(const boost::system::error_code& error)
{
  if (error != boost::asio::error::operation_aborted) {
    closeConnection();
  }
}

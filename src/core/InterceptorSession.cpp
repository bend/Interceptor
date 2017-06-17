#include "InterceptorSession.h"

#include "Defs.h"
#include "utils/Logger.h"

#include "socket/InboundConnection.h"
#include "http/HttpRequest.h"
#include "http/HttpBuffer.h"
#include "http/HttpReply.h"

#include "cache/generic_cache.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

InterceptorSession::InterceptorSession(const Config::ServerConfig* config,
                                       AbstractCacheHandler* cache,
                                       boost::asio::io_service& ioService)
  : m_config(config),
    m_cache(cache),
    m_ioService(ioService),
    m_iostrand(ioService),
    m_fsstrand(ioService),
    m_readTimer(ioService),
    m_writeTimer(ioService),
    m_state(CanSend)
{
  m_connection = std::make_shared<TcpInboundConnection>(m_ioService);
}

InterceptorSession::~InterceptorSession()
{
  LOG_DEBUG("InterceptorSession::~InterceptorSession()");
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

void InterceptorSession::postReply(HttpBufferPtr buffer)
{
  LOG_DEBUG("InterceptorSession::postReply()");
  m_ioService.post(
    m_iostrand.wrap(
      boost::bind(&InterceptorSession::sendNext, shared_from_this(), buffer)));
}

void InterceptorSession::sendNext(HttpBufferPtr buffer)
{
  LOG_DEBUG("InterceptorSession::sendNext()");
  {
    boost::mutex::scoped_lock lock(m_buffersMutex);
    m_buffers.push_back(buffer);

    if (m_state & CanSend) {
      auto v = m_buffers.front();
      m_buffers.pop_front();
      sendReply(v);
    }
  }
}

void InterceptorSession::sendReply(HttpBufferPtr buffer)
{
  LOG_DEBUG("InterceptorSession::sendReply()");

  if (m_connection) {
    startWriteTimer();
    m_state &= ~CanSend;

    if (buffer->flags() & Http::HttpBuffer::HasMore)
      m_ioService.post(
        m_fsstrand.wrap(buffer->nextCall()));

    m_connection->asyncWrite(buffer->m_buffers, m_iostrand.wrap
                             (boost::bind
                              (&InterceptorSession::handleTransmissionCompleted,
                               shared_from_this(),
                               buffer,
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred)
                             )
                            );
  }
}

void InterceptorSession::handleTransmissionCompleted(
  HttpBufferPtr buffer,
  const boost::system::error_code& error, size_t bytesTransferred)
{
  LOG_DEBUG("InterceptorSession::handleTransmissionCompleted()");
  stopWriteTimer();

  if (!error)  {
    LOG_DEBUG("Response sent ");
    m_state |= CanSend;

    {
      boost::mutex::scoped_lock lock(m_buffersMutex);

      if (!m_buffers.empty()) {

        auto v = m_buffers.front();
        m_buffers.pop_front();
        sendReply(v);
      }
    }

  } else {
    LOG_ERROR("Could not send reponse " << error.message());
    closeConnection();
  }

  if (buffer->flags() & Http::HttpBuffer::Closing) {
    closeConnection();
  }
}

void InterceptorSession::closeConnection()
{
  LOG_DEBUG("InterceptorSession::closeConnection()");

  if (m_state & Closing) {
    return;
  }

  m_state |= Closing;
  stopReadTimer();
  stopWriteTimer();
  m_state &= ~CanSend;
  m_ioService.post(
    m_iostrand.wrap(
      boost::bind(&InterceptorSession::doCloseConnection, shared_from_this())));
}

void InterceptorSession::doCloseConnection()
{
  if (m_connection) {
    m_connection->disconnect();
  }
}

void InterceptorSession::start()
{
  LOG_DEBUG("InterceptorSession::start()");

  // Avoid Slow Loris attacks, close connection if nothing read
  if (!(m_state & Closing) && m_connection) {
    startReadTimer();
    m_ioService.post(
      m_iostrand.wrap(
        boost::bind(&InterceptorSession::doRead, shared_from_this())));

  }
}

void InterceptorSession::doRead()
{
  m_connection->asyncReadSome(m_requestBuffer, sizeof(m_requestBuffer),
                              boost::bind(&InterceptorSession::handleHttpRequestRead, shared_from_this(),
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred)
                             );

}

void InterceptorSession::handleHttpRequestRead(const boost::system::error_code&
    error, size_t bytesTransferred)
{
  LOG_DEBUG("InterceptorSession::handleHttpRequestRead()");
  stopReadTimer();

  if (!error) {
    LOG_INFO("Request read from " << m_connection->ip());

    if (!m_request || m_request->completed() ) {
      m_request = std::make_shared<Http::HttpRequest>(shared_from_this(), m_cache);
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
    if (m_connection) {
      LOG_ERROR("Error reading request from " << m_connection->ip());
    } else {
      LOG_ERROR("Error reading request");
    }

  }
}

void InterceptorSession::startReadTimer()
{
  LOG_DEBUG("InterceptorSession::startReadTimer()");
  m_state |= Reading;
  LOG_DEBUG("Setting timeout to " << m_config->m_clientTimeout);
  m_readTimer.expires_from_now(boost::posix_time::seconds(
                                 m_config->m_clientTimeout));
  m_readTimer.async_wait
  (m_iostrand.wrap
   (boost::bind(&InterceptorSession::handleTimeout,
                shared_from_this(),
                ReadTimer,
                boost::asio::placeholders::error)));

}

void InterceptorSession::startWriteTimer()
{
  LOG_DEBUG("InterceptorSession::startWriteTimer()");
  m_state |= Sending;
  m_writeTimer.expires_from_now(boost::posix_time::seconds(
                                  m_config->m_serverTimeout));
  m_writeTimer.async_wait
  (m_iostrand.wrap
   (boost::bind(&InterceptorSession::handleTimeout,
                shared_from_this(),
                WriteTimer,
                boost::asio::placeholders::error)));
}

void InterceptorSession::stopReadTimer()
{
  LOG_DEBUG("InterceptorSession::stopReadTimer");
  m_state &= ~Reading;
  m_readTimer.cancel();
}

void InterceptorSession::stopWriteTimer()
{
  LOG_DEBUG("InterceptorSession::stopWriteTimer()");
  m_state &= ~Sending;
  m_writeTimer.cancel();
}

void InterceptorSession::handleTimeout(TimerType timerType,
                                       const boost::system::error_code& error)
{
  LOG_DEBUG("InterceptorSession::handleTimeout()");

  if (error != boost::asio::error::operation_aborted) {
    if ((timerType == ReadTimer) && ((m_state & Reading & Sending)
                                     || m_buffers.size() > 0)) {
      startReadTimer();  // We are writing something on the socket, so start the timer again
    } else {
      closeConnection();
    }
  }
}

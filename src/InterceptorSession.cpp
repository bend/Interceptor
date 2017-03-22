#include "InterceptorSession.h"

#include "Defs.h"
#include "InboundConnection.h"
#include "HttpRequest.h"
#include "HttpReply.h"
#include "Logger.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

InterceptorSession::InterceptorSession(boost::asio::io_service& ioService)
  : m_ioService(ioService)
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

void InterceptorSession::start() 
{
  // Avoid Slow Loris attacks, close connection after 2min if
  // no preamble received
  // Read preamble Size from mobile
  InterceptorSessionPtr isp = shared_from_this();

  if (m_connection) {
	/*
		m_conn
  ection->asyncReadUntil(m_requestBuffer4096, boost::regex("^(\r\n)"),
									 boost::bind(&InterceptorSession::handleHttpRequestRead, isp,
									        	 boost::asio::placeholders::error,
												 boost::asio::placeholders::bytes_transferred)
									);
  */
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
	  trace("info") << "Bytes transfered " << bytesTransferred;

	  if(!m_request) {
		// Create Request
		m_request = std::make_shared<HttpRequest>(shared_from_this());
	  }
	  m_request->appendData(m_requestBuffer, bytesTransferred);
	  if(!m_request->headersReceived())
		start();
	  else  {
		m_reply = std::make_shared<HttpReply>(m_request);
		m_reply->process();
	  }
	} else {
		trace("error") << "Error reading request from " << m_connection->ip();
	}
}

void InterceptorSession::parseHttpRequest(const std::string& request)
{
}

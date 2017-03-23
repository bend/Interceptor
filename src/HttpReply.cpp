#include "HttpReply.h"

#include "HttpRequest.h"
#include "InterceptorSession.h"
#include "InboundConnection.h"

#include <boost/bind.hpp>


HttpReply::HttpReply(HttpRequestPtr request)
  : m_request(request)
{
}


void HttpReply::process()
{
  if ( !m_request->headersReceived() )
    // TODO error
    return;
  m_request->parse();
  switch (m_request->method()) {
  case HttpRequest::GET:
    handleGetRequest();
    break;
  case HttpRequest::POST:
    break;
  default:
    break;
  }
}

void HttpReply::handleGetRequest()
{
  std::stringstream response;
  response << m_request->httpVersion() << 200 << " OK\r\n";
  response << "Content-Type: text/html; charset=UTF-8\r\n";
  response << "Server: Interceptor/0.1 (Unix) (Ubuntu)\r\n";
  std::string html = "<html><head><title>Test page</title></head><body>Hello world, this is Interceptor 0.1 talking</body></html>";
  response << "Content-length: " << html.length() << "\r\n";
  response << "\r\n";
  response << html;
  m_response = response.str();
  m_request->session()->connection()->asyncWrite(m_response.data(), m_response.length(), boost::bind(
        &HttpReply::handleHttpResponseSent, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void HttpReply::handleHttpResponseSent(const boost::system::error_code& error, size_t bytesTransferred)
{
}

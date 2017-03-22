#include "HttpRequest.h"

#include "HttpHeaders.h"
#include "Logger.h"

#include <boost/algorithm/string.hpp>

HttpRequest::HttpRequest(InterceptorSessionPtr session)
  : m_session(session),
  m_headers(nullptr)
{
}

HttpRequest::~HttpRequest()
{
  delete m_headers;
}

void HttpRequest::appendData(const unsigned char* data, size_t length)
{

}

bool HttpRequest::headersReceived() const
{
  return true; //TODO
}

Host HttpRequest::host() const
{ 
  std::string host = *m_headers->getHeader("Host");
  return host;
}

HttpRequest::Method HttpRequest::method() const
{
  return m_method;
}

std::string HttpRequest::index() const
{
  return m_index;
}

std::string HttpRequest::httpVersion() const
{
  return m_httpVersion;
}

void HttpRequest::parse()
{
  size_t pos = m_request.find_first_of("\r\n");
  if (pos == std::string::npos) 
  {
	trace("error") << "HttpRequest missing separator.. aborting";
	//TODO handle error
	return;
  }

  std::string get = m_request.substr(0, pos);
  m_request = m_request.substr(pos);

  std::vector<std::string> getParts;
  boost::split(getParts, get , boost::is_any_of(" "));

  if (getParts.size() != 3) 
  {
	trace("error") << "Missing Method part";
	//TODO handle error
	return;
  }

  parseMethod(getParts[0]);
  m_index = getParts[1];
  m_httpVersion = getParts[2]; //TODO parse and check
  m_headers = new HttpHeaders(m_request);
}

void HttpRequest::parseMethod(const std::string& method)
{
  if(method == "GET")
	m_method = Method::GET;
  else if (method == "HEAD")
	m_method = HEAD;
  else if(method == "POST")
	m_method = POST;
}


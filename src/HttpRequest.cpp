#include "HttpRequest.h"

#include "HttpHeaders.h"
#include "Logger.h"
#include "InterceptorSession.h"

#include <boost/algorithm/string.hpp>
#include <regex>

HttpRequest::HttpRequest(InterceptorSessionPtr session)
  : m_session(session),
    m_headers(nullptr),
    m_completed(false),
    m_host(""),
	m_status(Http::ErrorCode::Ok)
{
}

HttpRequest::~HttpRequest()
{
  delete m_headers;
}

void HttpRequest::appendData(const unsigned char* data, size_t length)
{
  m_request.append(std::string(data, data + length));
}

bool HttpRequest::headersReceived() const
{
  return m_request.find("\r\n\r\n") != std::string::npos;
}

Host HttpRequest::host() const
{
  return m_host;
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

InterceptorSessionPtr HttpRequest::session() const
{
  return m_session;
}

bool HttpRequest::completed() const
{
  return m_completed;
}

void HttpRequest::setCompleted(bool completed)
{
  m_completed = completed;
}

void HttpRequest::setStatus(Http::ErrorCode error)
{
  m_status = error;
}

Http::ErrorCode HttpRequest::status() const
{
  return m_status;
}

void HttpRequest::parse()
{
  size_t pos = m_request.find_first_of("\r\n");
  if (pos == std::string::npos) {
    trace("error") << "HttpRequest missing separator.. aborting";
	setStatus(Http::ErrorCode::BadRequest);
    return;
  }
  std::string get = m_request.substr(0, pos);
  m_request = m_request.substr(pos);
  std::vector<std::string> getParts;
  boost::split(getParts, get , boost::is_any_of(" "));
  if (getParts.size() != 3) {
    trace("error") << "Missing Method part";
	setStatus(Http::ErrorCode::BadRequest);
    return;
  }
  parseMethod(getParts[0]);
  switch (m_method) {
  case Method::GET: {
      size_t st = getParts[1].find("?");
      if (st != std::string::npos) {
        parseParameters(getParts[1].substr(st));
        getParts[1] = getParts[1].substr(0, st);
      }
    }
    break;
  case Method::POST:
    //pase from form
    break;
  default:
    break;
  }
  m_index = getParts[1];
  m_httpVersion = getParts[2]; //TODO parse and check
  trace("debug") << get;
  m_headers = new HttpHeaders(m_request);

  // parse host
  const std::string* host = m_headers->getHeader("Host");

  if (!host )
    //TODO error
    return;

  size_t spos = host->find(":");
  m_host = host->substr(0, spos);
}

void HttpRequest::parseMethod(const std::string& method)
{
  if (method == "GET")
    m_method = Method::GET;
  else if (method == "HEAD")
    m_method = HEAD;
  else if (method == "POST")
    m_method = POST;
}

void HttpRequest::parseParameters(const std::string& params)
{
  trace("debug") << "Parsing parameters " << params;
}

bool HttpRequest::hasMatchingSite() const
{
  return matchingSite() != nullptr;
}

const Config::ServerConfig::Site* HttpRequest::matchingSite() const
{
  const std::string host = m_host;
  for (const auto& site : session()->config()->m_sites) {
    if (site->m_host == host)
      return site;
  }
  return nullptr;
}

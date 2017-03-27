#include "HttpReply.h"

#include "HttpRequest.h"
#include "HttpHeaders.h"
#include "InterceptorSession.h"
#include "InboundConnection.h"
#include "Utils.h"
#include "Config.h"


#include <boost/bind.hpp>


HttpReply::HttpReply(HttpRequestPtr request)
  : m_request(request),
    m_replyHeaders(nullptr)
{
}

HttpReply::~HttpReply()
{
  delete m_replyHeaders;
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
  if (m_replyHeaders)
    delete m_replyHeaders;
  m_replyHeaders = new HttpHeaders();
  m_replyHeaders->addGeneralHeaders();
  std::stringstream response;
  std::string page = SConfig.docRoot() + m_request->index();
  size_t pageLength = 0;
  unsigned char* pageData;
  if (!Utils::readFile(page, &pageData, pageLength)) {
    //TODO error
    response << m_request->httpVersion() << " " << 404 << " Not  Found\r\n";
    send(response);
    return;
  }
  response << m_request->httpVersion() << " " << 200 << " OK\r\n";
  m_replyHeaders->addHeader("Content-Type", Utils::getMimeType(page));
  m_replyHeaders->addHeader("Content-Length", pageLength);
  m_replyHeaders->serialize(response);
  send(response);
  InterceptorSession::Packet* packet = new InterceptorSession::Packet();
  packet = new InterceptorSession::Packet(pageData, pageLength);
  m_request->session()->postResponse(packet);
  m_request->setCompleted(true);
}

void HttpReply::send(std::stringstream& stream)
{
  InterceptorSession::Packet* packet = new InterceptorSession::Packet();
  size_t size = stream.str().length();
  unsigned char* data = new unsigned char[size];
  memcpy(data, stream.str().data(), size);
  packet->m_data = data;
  packet->m_size = size;
  m_request->session()->postResponse(packet);
}


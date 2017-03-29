#include "HttpReply.h"

#include "HttpRequest.h"
#include "HttpHeaders.h"
#include "InterceptorSession.h"
#include "Utils.h"
#include "Config.h"


#include <boost/bind.hpp>


HttpReply::HttpReply(HttpRequestPtr request)
  : m_request(request),
    m_replyHeaders(nullptr),
    m_status(Http::ErrorCode::Ok),
    m_close(false)
{
}

HttpReply::~HttpReply()
{
  delete m_replyHeaders;
}


void HttpReply::process()
{
  std::stringstream stream;
  if (m_replyHeaders) {
    delete m_replyHeaders;
  }

  m_replyHeaders = new HttpHeaders();
  m_replyHeaders->addGeneralHeaders();

  if ( !m_request->headersReceived() )
    std::stringstream stream;
  buildErrorResponse(Http::ErrorCode::BadRequest, stream, true);
  return;

  m_request->parse();

  if (m_request->status() != Http::ErrorCode::Ok) {
    m_replyHeaders->fillFrom(m_request->m_headers);
    buildErrorResponse(m_request->status(), stream, true);
    return;
  }

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
  std::stringstream stream;

  if (!m_request->hasMatchingSite()) {
    buildErrorResponse(Http::NotFound, stream, true);
    return;
  }

  const Config::ServerConfig::Site* site = m_request->matchingSite();

  std::string page;
  size_t pageLength = 0;

  if ( m_request->index() == "" || m_request->index() == "/") {
    // This request does not contain a filename, we will use a page from try-file
    std::vector<std::string> tryFiles = site->m_tryFiles;
    bool found = false;
    for (const auto& index : tryFiles) {
      page = site->m_docroot + index;

      if (!Utils::readFile(page, stream, pageLength)) {
        found = true;
        m_replyHeaders->addHeader("Content-Type", Utils::getMimeType(page));
        break;
      }
    }

    if (!found) {
      buildErrorResponse(Http::ErrorCode::NotFound, stream);
    }

  } else {
    // This request contains the filename, hence we should not try a filename from the list of try-files
    page = site->m_docroot + m_request->index();

    if (!Utils::readFile(page, stream, pageLength)) {
      buildErrorResponse(Http::ErrorCode::NotFound, stream);
      return;
    } else
      m_replyHeaders->addHeader("Content-Type", Utils::getMimeType(page));

  }

  m_request->setCompleted(true);

  post(stream);
}

void HttpReply::post(std::stringstream& stream)
{
  /*
  InterceptorSession::Packet* packet = new InterceptorSession::Packet();
  size_t size = stream.str().length();
  unsigned char* data = new unsigned char[size];
  memcpy(data, stream.str().data(), size);
  packet->m_data = data;
  packet->m_size = size;
  m_request->session()->postReply(shared_from_this());
  */
}

void HttpReply::chunkResponse(std::stringstream& stream)
{

}

void HttpReply::encodeResponse(std::stringstream& stream)
{

}

void HttpReply::buildHeader()
{
  std::stringstream stream;
  stream << m_request->httpVersion() << " ";
  Http::stringValue(Http::ErrorCode::Ok, stream);

  //if(m_chunked)
  //m_replyHeaders->addHeader("Content-Length", pageLength);

  m_replyHeaders->serialize(stream);
}

void HttpReply::buildErrorResponse(Http::ErrorCode error, std::stringstream& stream, bool closeConnection)
{
  bool found = false;
  size_t pageLength = 0;

  const ErrorPageMap& map = m_request->hasMatchingSite() ?
                            m_request->matchingSite()->m_errorPages : m_request->session()->config()->m_errorPages;

  if (map.count(std::to_string(error)) > 0 ) {
    std::string url = map.at(std::to_string(error));

    if (Utils::readFile(url, stream, pageLength)) {
      found = true;
      m_replyHeaders->addHeader("Content-Type", Utils::getMimeType(url));
    }
  }

  if (!found) {
    stream << "<html> <body> ";
    Http::stringValue(error, stream);
    stream << " </body>  </html>";
  }

  if ( closeConnection) {
    m_replyHeaders->addHeader("Connection", "close");
    m_close = closeConnection;
  }

  m_request->setCompleted(true);
  m_status = error;

  post(stream);

}


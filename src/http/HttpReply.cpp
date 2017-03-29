#include "HttpReply.h"

#include "HttpRequest.h"
#include "HttpHeaders.h"
#include "InterceptorSession.h"
#include "Utils.h"
#include "Config.h"
#include "Logger.h"

#include <boost/bind.hpp>


HttpReply::HttpReply(HttpRequestPtr request)
  : m_request(request),
    m_replyHeaders(nullptr),
    m_status(Http::ErrorCode::Ok)
{
  setFlag(Flag::Chunked, true);
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

  if ( !m_request->headersReceived() ) {
    std::stringstream stream;
    buildErrorResponse(Http::ErrorCode::BadRequest, stream, true);
    return;
  }

  Http::ErrorCode status =  m_request->parse();

  if (status != Http::ErrorCode::Ok) {
    if (m_request->m_headers)
      m_replyHeaders->fillFrom(m_request->m_headers);

    buildErrorResponse(status, stream, true);
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

const std::vector<boost::asio::const_buffer>& HttpReply::buffers() const
{
  return m_buffers;
}

void HttpReply::setFlag(Flag flag, bool value)
{
  m_flags.set(flag, value);
}

bool HttpReply::getFlag(Flag flag) const
{
  return m_flags.test(flag);
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

  if ( m_request->index() == ""
       || m_request->index() == "/") {
    // This request does not contain a filename, we will use a page from try-file
    std::vector<std::string> tryFiles = site->m_tryFiles;
    bool found = false;

    for (const auto& index : tryFiles) {
      page = site->m_docroot + index;

      if (Utils::readFile(page, stream, pageLength)) {
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
  if (getFlag(Flag::Chunked))
    chunkResponse(stream);

  m_buffers.clear();
  m_buffers.push_back({});

  const std::string& resp = stream.str();
  char* buff = new char[resp.length()];

  memcpy(buff, resp.data(), resp.length());
  m_buffers.push_back(boost::asio::buffer(buff, resp.length()));
  buildHeaders();

  m_request->session()->postReply(shared_from_this());
}

void HttpReply::chunkResponse(std::stringstream& stream)
{
  std::string str = stream.str();
  stream.str(std::string());
  stream << std::hex << str.length() << "\r\n";
  stream << str << "\r\n";
  stream << 0 << "\r\n\r\n";
}

void HttpReply::encodeResponse(std::stringstream& stream)
{

}

void HttpReply::buildHeaders()
{
  std::stringstream stream;
  stream << m_request->httpVersion() << " ";
  Http::stringValue(Http::ErrorCode::Ok, stream);

  if (getFlag(Flag::Chunked))
    m_replyHeaders->addHeader("Transfer-Encoding", "Chunked");
  else
    m_replyHeaders->addHeader("Content-Length", boost::asio::buffer_size(m_buffers[1]));

  m_replyHeaders->serialize(stream);
  const std::string& resp = stream.str();
  char* buff = new char[resp.length()];
  memcpy(buff, resp.data(), resp.length());
  m_buffers[0] = boost::asio::buffer(buff, resp.length());
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
    setFlag(Flag::Closing, closeConnection);
  }

  m_request->setCompleted(true);
  m_status = error;

  post(stream);

}


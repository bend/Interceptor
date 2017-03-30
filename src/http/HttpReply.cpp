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
  setFlag(Flag::ChunkedEncoding, true);
  setFlag(Flag::GzipEncoding, true);
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
  char* bb = new char[stream.str().length()]();
  memcpy(bb, stream.str().data(), stream.str().length());
  std::vector<boost::asio::const_buffer> buffers;
  buffers.push_back(boost::asio::buffer(bb, stream.str().length()));

  if (getFlag(Flag::GzipEncoding)) {
    encodeResponse(buffers);
  }

  if (getFlag(Flag::ChunkedEncoding)) {
    chunkResponse(buffers);
  }

  m_buffers.clear();
  m_buffers.push_back({});

  m_buffers.insert(m_buffers.end(), buffers.begin(), buffers.end());
  buildHeaders();
  trace("info") << m_request->toString() << " " << m_status;

  m_request->session()->postReply(shared_from_this());
}

bool HttpReply::chunkResponse(std::vector<boost::asio::const_buffer>& buffers)
{
  size_t size = 0;

  for (auto& buffer : buffers)
    size += boost::asio::buffer_size(buffer);

  std::stringstream stream;
  stream << std::hex << size << "\r\n";

  char* header = new char[stream.str().length()]();
  memcpy(header, stream.str().data(), stream.str().length());
  buffers.insert(buffers.begin(), boost::asio::buffer(header, stream.str().length()));

  stream.str("\r\n0\r\n\r\n");
  char* footer = new char[stream.str().length()]();
  memcpy(footer, stream.str().data(), stream.str().length());
  buffers.push_back(boost::asio::buffer(footer, stream.str().length()));

  return true;
}

bool HttpReply::encodeResponse(std::vector<boost::asio::const_buffer>& buffers)
{
  std::vector<boost::asio::const_buffer> result;
  initGzip();
  unsigned int i = 0;

  for (auto& buffer : buffers) {
    m_gzip.avail_in = boost::asio::buffer_size(buffer);
    m_gzip.next_in = (unsigned char*) boost::asio::detail::buffer_cast_helper(buffer);

    unsigned char out[16 * 1024];

    do {
      m_gzip.next_out = out;
      m_gzip.avail_out = sizeof(out);

      int res = 0;
      res = deflate(&m_gzip,
                    (i == buffers.size() - 1) ?
                    Z_FINISH : Z_NO_FLUSH);
      assert(res != Z_STREAM_ERROR);

      unsigned have = sizeof(out) - m_gzip.avail_out;

      if (have) {
        result.push_back(buf(std::string((char*)out, have)));
      }
    } while (m_gzip.avail_out == 0);

    ++i;

  }

  deflateEnd(&m_gzip);
  m_gzipBusy = false;

  buffers.clear();
  buffers.insert(buffers.begin(), result.begin(), result.end());

  return true;
}

void HttpReply::buildHeaders()
{

  std::stringstream stream;
  stream << m_request->httpVersion() << " ";
  Http::stringValue(m_status, stream);

  if (getFlag(Flag::ChunkedEncoding))
    m_replyHeaders->addHeader("Transfer-Encoding", "chunked");
  else
    m_replyHeaders->addHeader("Content-Length", boost::asio::buffer_size(m_buffers[1]));

  if (getFlag(Flag::GzipEncoding))
    m_replyHeaders->addHeader("Content-Encoding", "gzip");

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

void HttpReply::initGzip()
{
  m_gzip.zalloc = Z_NULL;
  m_gzip.zfree = Z_NULL;
  m_gzip.opaque = Z_NULL;
  m_gzip.next_in = Z_NULL;
  int r = 0;

  r = deflateInit2(&m_gzip, Z_DEFAULT_COMPRESSION,
                   Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);

  m_gzipBusy = true;
  assert(r == Z_OK);
}

boost::asio::const_buffer HttpReply::buf(const std::string& s)
{
  m_bufs.push_back(s);
  return boost::asio::buffer(m_bufs.back());
}


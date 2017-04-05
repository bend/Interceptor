#include "HttpReply.h"

#include "core/InterceptorSession.h"
#include "HttpRequest.h"
#include "HttpHeaders.h"
#include "FileUtils.h"
#include "Logger.h"

#include <boost/bind.hpp>


HttpReply::HttpReply(HttpRequestPtr request)
  : m_request(request),
    m_replyHeaders(nullptr),
    m_status(Http::Code::Ok),
    m_contentLength(0),
    m_gzipBusy(false)
{
}

HttpReply::~HttpReply()
{
  delete m_replyHeaders;

  for (auto b : m_bufs2) {
    delete[] b;
  }
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
    buildErrorResponse(Http::Code::BadRequest, stream, true);
    return;
  }

  Http::Code status =  m_request->parse();

  if (status != Http::Code::Ok) {
    if (m_request->m_headers) {
      m_replyHeaders->fillFrom(m_request->m_headers);
    }

    buildErrorResponse(status, stream, true);
    return;
  }

  setFlag(Flag::ChunkedEncoding, m_request->supportsChunking());
  setFlag(Flag::GzipEncoding, m_request->supportsCompression());

  switch (m_request->method()) {
    case Http::Method::GET:
    case Http::Method::HEAD:
      handleRetrievalRequest(m_request->method());
      break;

    case Http::Method::POST:
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

void HttpReply::handleRetrievalRequest(Http::Method method)
{
  std::stringstream stream;

  if (!m_request->hasMatchingSite()) {
    buildErrorResponse(Http::Code::NotFound, stream, true);
    return;
  }

  const SiteConfig* site = m_request->matchingSite();

  requestFileContents(method, site, stream);
}

bool HttpReply::requestFileContents(Http::Method method, const SiteConfig* site, std::stringstream& stream)
{
  std::string page;
  bool found = false;
  size_t bytes = 0;

  if ( m_request->index() == ""
       || m_request->index() == "/") {
    // This request does not contain a filename, we will use a page from try-file
    std::vector<std::string> tryFiles = site->m_tryFiles;

    for (const auto& index : tryFiles) {
      page = site->m_docroot + index;

      if (FileUtils::exists(page)) {
        found = true;
        break;
      }
    }
  } else {
    // This request contains the filename, hence we should not try a filename from the list of try-files
    page = site->m_docroot + m_request->index();

    if (FileUtils::exists(page)) {
      found = true;
    }
  }

  if (!found) {
    buildErrorResponse(Http::Code::NotFound, stream);
    return false;
  }

  // page found
  if ( method == Http::Method::GET) {
    bool ret;

    if (m_request->partialRequest()) {
      ret = requestPartialFileContents(page, stream, bytes);
    } else {
      FileUtils::fileSize(page, bytes);
      // if(bytes > MAX_CHUNK_SIZE) {
      // File is too big to be sent at once, we will send it in multiple times to
      // avoid consuming to much memory
      //	return requestLargeFileContents(page, stream);
      //} else
      ret = FileUtils::readFile(page, stream, bytes);
    }

    if (!ret) {
      buildErrorResponse(Http::Code::NotFound, stream);
      return false;
    } else {
      setMimeType(page);
    }
  } else if (method == Http::Method::HEAD) {
    if (!FileUtils::fileSize(page, bytes)) {
      buildErrorResponse(Http::Code::NotFound, stream);
      return false;
    } else {
      setMimeType(page);
    }

  } else {
    return false;
  }

  m_contentLength = bytes;

  m_request->setCompleted(true);

  post(stream);

  return true;
}

bool HttpReply::requestPartialFileContents(const std::string& page, std::stringstream& stream, size_t& bytes)
{
  std::vector<uint64_t> sizes;
  std::tuple<int64_t, int64_t> range = m_request->getRangeRequest();
  bool ret = FileUtils::readFile(page, range, stream, sizes);

  if (!ret) {
    return ret;
  }

  m_replyHeaders->addHeader("Content-Range", "bytes " + std::to_string(sizes[0]) + "-"
                            + std::to_string(sizes[1]) + "/" + std::to_string(sizes[2]));
  bytes = sizes[1] - sizes[0] + 1;
  m_status = Http::Code::PartialContent;

  return ret;
}


bool HttpReply::requestLargeFileContents(const std::string& page, std::stringstream& stream)
{
  return true;
}

void HttpReply::post(std::stringstream& stream)
{

  for (auto b : m_bufs2) {
    delete[] b;
  }

  m_bufs2.clear();
  m_bufs.clear();
  m_buffers.clear();

  std::vector<boost::asio::const_buffer> buffers;
  buffers.push_back(buf(std::string(stream.str())));

  if (canEncodeResponse()) {
    encodeResponse(buffers);
  }

  if (canChunkResponse()) {
    chunkResponse(buffers);
  }

  m_buffers.clear();

  if (!getFlag(HeadersSent))
    m_buffers.push_back({}); // emtpy place for headers

  m_buffers.insert(m_buffers.end(), buffers.begin(), buffers.end());

  if (!getFlag(HeadersSent)) {
    buildHeaders();
    trace("info") << m_request->queryString() << " " << (int) m_status;
  }

  m_request->session()->postReply(shared_from_this());
}

bool HttpReply::chunkResponse(std::vector<boost::asio::const_buffer>& buffers)
{
  size_t size = 0;

  for (auto& buffer : buffers) {
    size += boost::asio::buffer_size(buffer);
  }

  std::stringstream stream;
  stream << std::hex << size << "\r\n";

  char* header = new char[stream.str().length()]();
  memcpy(header, stream.str().data(), stream.str().length());
  buffers.insert(buffers.begin(), buf(header, stream.str().length()));

  stream.str("\r\n0\r\n\r\n");
  char* footer = new char[stream.str().length()]();
  memcpy(footer, stream.str().data(), stream.str().length());
  buffers.push_back(buf(footer, stream.str().length()));

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
      m_contentLength += have;

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
  stream << "HTTP/" << m_request->httpVersion() << " ";
  Http::stringValue(m_status, stream);

  if (canChunkResponse()) {
    m_replyHeaders->addHeader("Transfer-Encoding", "chunked");
  } else {
    m_replyHeaders->addHeader("Content-Length", m_contentLength);
  }

  if (canEncodeResponse()) {
    m_replyHeaders->addHeader("Content-Encoding", "gzip");
  }

  if (getFlag(Flag::Closing)) {
    m_replyHeaders->addHeader("Connection", "close");
  } else {
    m_replyHeaders->addHeader("Connection", "keep-alive");
  }

  m_replyHeaders->serialize(stream);
  const std::string& resp = stream.str();
  m_buffers[0] = buf(std::string(resp));
}

void HttpReply::buildErrorResponse(Http::Code error, std::stringstream& stream, bool closeConnection)
{
  bool found = false;
  size_t bytes = 0;

  const ErrorPageMap& map = m_request->hasMatchingSite() ?
                            m_request->matchingSite()->m_errorPages : m_request->session()->config()->m_errorPages;

  if (map.count(std::to_string((int)error)) > 0 ) {
    std::string url = map.at(std::to_string((int)error));

    if (FileUtils::readFile(url, stream, bytes)) {
      found = true;
      setMimeType(url);
    }
  }

  if (!found) {
    stream << "<html> <body><h1>";
    Http::stringValue(error, stream);
    stream << " </h1></body></html>";
    bytes = stream.str().length();
  }

  setFlag(Flag::Closing, closeConnection);

  m_request->setCompleted(true);
  m_status = error;
  m_contentLength = bytes;

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

boost::asio::const_buffer HttpReply::buf(char* buf, size_t s)
{
  m_bufs2.push_back(buf);
  return boost::asio::buffer(m_bufs2.back(), s);
}

void HttpReply::setMimeType(const std::string& filename)
{
  m_replyHeaders->addHeader("Content-Type", FileUtils::mimeType(filename));
  auto tuple = FileUtils::generateCacheData(filename);

  if (std::get<0>(tuple).length() > 0) {
    m_replyHeaders->addHeader("ETag", std::get<0>(tuple));
  }

  if (std::get<1>(tuple).length() > 0) {
    m_replyHeaders->addHeader("Last-Modified", std::get<1>(tuple));
  }

  if (!m_request->hasMatchingSite()) {
    setFlag(Flag::GzipEncoding, false);
    return;
  }

  auto site = m_request->matchingSite();

  if (site->m_gzip.count("all") == 0 && site->m_gzip.count(FileUtils::extension(filename)) == 0) {
    setFlag(Flag::GzipEncoding, false);
  }

}

bool HttpReply::canChunkResponse() const
{
  return getFlag(Flag::ChunkedEncoding) && m_request->method() != Http::Method::HEAD
         && m_status != Http::Code::PartialContent;
}

bool HttpReply::canEncodeResponse() const
{
  return getFlag(Flag::GzipEncoding) && m_request->method() != Http::Method::HEAD;
}

#include "HttpReply.h"

#include "core/InterceptorSession.h"
#include "HttpRequest.h"
#include "HttpHeaders.h"
#include "HttpBuffer.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"
#include "cache/generic_cache.h"

#include <boost/bind.hpp>
#include <algorithm>

namespace Http {

  HttpReply::HttpReply(HttpRequestPtr request)
    : m_request(request),
      m_replyHeaders(nullptr),
      m_status(Code::Ok),
      m_contentLength(0),
      m_gzipBusy(false)
  {
  }

  HttpReply::~HttpReply()
  {
    LOG_DEBUG("HttpReply::~HttpReply()");
    delete m_replyHeaders;
  }

  void HttpReply::process()
  {
    LOG_DEBUG("HttpReply::process()");
    std::stringstream stream;
    m_httpBuffer = std::make_shared<HttpBuffer>();

    if (m_replyHeaders) {
      delete m_replyHeaders;
    }

    m_replyHeaders = new HttpHeaders();
    m_replyHeaders->addGeneralHeaders();

    if ( !m_request->headersReceived() ) {
      std::stringstream stream;
      buildErrorResponse(Code::BadRequest, stream, true);
      return;
    }

    Code status =  m_request->parse();

    if (status != Code::Ok) {
      if (m_request->m_headers) {
        m_replyHeaders->fillFrom(m_request->m_headers);
      }

      buildErrorResponse(status, stream, true);
      return;
    }

    setFlag(Flag::ChunkedEncoding, m_request->supportsChunking());

#ifdef ENABLE_GZIP
    setFlag(Flag::GzipEncoding, m_request->supportsCompression());
#endif // ENABLE_GZIP

    switch (m_request->method()) {
      case Method::GET:
      case Method::HEAD:
        handleRetrievalRequest(m_request->method());
        break;

      case Method::POST:
        break;

      default:
        break;
    }
  }

  void HttpReply::setFlag(Flag flag, bool value)
  {
    m_flags.set(flag, value);
  }

  bool HttpReply::getFlag(Flag flag) const
  {
    return m_flags.test(flag);
  }

  void HttpReply::handleRetrievalRequest(Method method)
  {
    LOG_DEBUG("HttpReply::handleRetrievalRequest()");
    std::stringstream stream;

    if (!m_request->hasMatchingSite()) {
      buildErrorResponse(Code::NotFound, stream, true);
      return;
    }

    const SiteConfig* site = m_request->matchingSite();

    requestFileContents(method, site, stream);
  }

  bool HttpReply::requestFileContents(Method method, const SiteConfig* site,
                                      std::stringstream& stream)
  {
    LOG_DEBUG("HttpReply::requestFileContents()");
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
      // This request contains the filename, hence we should
      // not try a filename from the list of try-files
      page = site->m_docroot + m_request->index();

      if (FileUtils::exists(page)) {
        found = true;
      }
    }

    if (!found) {
      buildErrorResponse(Code::NotFound, stream);
      return false;
    }

    // page found
    if ( method == Method::GET) {
      Code ret;

      if (m_request->partialRequest()) {
        ret = requestPartialFileContents(page, stream, bytes);
      } else {
        if (!m_request->cacheHandler()->size(page, bytes)) {
          buildErrorResponse(Code::InternalServerError, stream);
          return false;
        }

        if (bytes > MAX_CHUNK_SIZE) {
          // File is too big to be sent at once, we will send it in multiple times to
          // avoid consuming to much memory
          setMimeType(page);
          return requestLargeFileContents(page, 0, bytes);
        } else {
          ret = m_request->cacheHandler()->read(page, stream, bytes);
        }
      }

      if (ret != Code::Ok) {
        buildErrorResponse(ret, stream);
        return false;
      } else {
        setMimeType(page);
      }
    } else if (method == Method::HEAD) {
      if (m_request->cacheHandler()->size(page, bytes)) {
        buildErrorResponse(Code::NotFound, stream);
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

  Code HttpReply::requestPartialFileContents(const std::string& page,
      std::stringstream& stream, size_t& bytes)
  {
    LOG_DEBUG("HttpReply::requestPartialFileContents()");
    std::tuple<int64_t, int64_t> range = m_request->getRangeRequest();
    int64_t from = std::get<0>(range);
    int64_t to = std::get<1>(range);
    size_t total = 0;
    Code ret = FileUtils::calculateBounds(page, from, to);

    if (ret != Code::Ok) {
      return ret;
    }

    ret = FileUtils::readFile(page,	from, to , stream, total);

    if (ret != Code::Ok) {
      return ret;
    }

    bytes = to - from + 1;

    m_replyHeaders->addHeader("Content-Range",
                              "bytes " + std::to_string(from) + "-"
                              + std::to_string(to) + "/" + std::to_string(total));
    m_status = Code::PartialContent;

    return ret;
  }

  bool HttpReply::requestLargeFileContents(const std::string& page, size_t from,
      size_t totalBytes)
  {
    LOG_DEBUG("HttpReply::requestLargeFileContents()");
    boost::mutex::scoped_lock lock(
      m_mutex); //needed to be sure that previous call is completed
    size_t bytes;
    size_t to = std::min((size_t) from + MAX_CHUNK_SIZE, totalBytes - 1);
    m_contentLength = totalBytes;
    setFlag(LargeFileRequest, true);

    m_httpBuffer = std::make_shared<HttpBuffer>();
    std::stringstream stream;

    if (FileUtils::readFile(page, from, to, stream, bytes) == Code::Ok) {
      if (to == totalBytes - 1) {
        m_request->setCompleted(true);
        post(stream);
        return true;
      } else {
        from = to + 1;
        m_httpBuffer->m_nextCall = std::bind(&HttpReply::requestLargeFileContents,
                                             shared_from_this(), page, from , totalBytes);
        m_httpBuffer->m_flags |= HttpBuffer::HasMore;
        post(stream);
      }

    } else {
      return false;
    }

    return true;
  }

  void HttpReply::post(std::stringstream& stream)
  {
    LOG_DEBUG("HttpReply::post()");
    std::vector<boost::asio::const_buffer> buffers;

    if (!getFlag(HeadersSent))
      m_httpBuffer->m_buffers.push_back({}); // emtpy place for headers

    buffers.push_back(buf(m_httpBuffer, std::string(stream.str())));

#ifdef ENABLE_GZIP

    if (canEncodeResponse()) {
      encodeResponse(m_httpBuffer, buffers);
    }

#endif // ENABLE_GZIP

    // We chunk only in the case that no header has been sent or if it's the last frame
    if (canChunkResponse()) {
      chunkResponse(m_httpBuffer, buffers);
    }

    if (!getFlag(HeadersSent)) {
      buildHeaders(m_httpBuffer);
      LOG_INFO(m_request->queryString() << " " << (int) m_status);
      setFlag(HeadersSent, true);
    }

    m_httpBuffer->m_buffers.insert(m_httpBuffer->m_buffers.end(), buffers.begin(),
                                   buffers.end());

    if (getFlag(Closing)) {
      m_httpBuffer->m_flags |= HttpBuffer::Closing;
    }

    auto session = m_request->session();

    if (session) {
      session->postReply(m_httpBuffer);
    }

    m_httpBuffer.reset();
  }

  bool HttpReply::chunkResponse(HttpBufferPtr httpBuffer,
                                std::vector<boost::asio::const_buffer>& buffers)
  {
    LOG_DEBUG("HttpReply::chunkResponse()");
    size_t size = 0;

    // We cannot take the total Content Length here because the gzip compression changed that
    // Length, so we need to recalculate it
    for (auto& buffer : buffers) {
      size += boost::asio::buffer_size(buffer);
    }

    std::stringstream stream;
    stream << std::hex << size << "\r\n";

    char* header = new char[stream.str().length()]();
    memcpy(header, stream.str().data(), stream.str().length());

    buffers.insert(buffers.begin(), buf(httpBuffer, header, stream.str().length()));

    stream.str("\r\n");
    char* crlf = new char[stream.str().length()]();
    memcpy(crlf, stream.str().data(), stream.str().length());
    buffers.push_back(buf(httpBuffer, crlf, stream.str().length()));

    if (!getFlag(LargeFileRequest) || m_request->completed()) {
      stream.str("0\r\n\r\n");
      char* footer = new char[stream.str().length()]();
      memcpy(footer, stream.str().data(), stream.str().length());
      buffers.push_back(buf(httpBuffer, footer, stream.str().length()));
    }

    return true;
  }

#ifdef ENABLE_GZIP
  bool HttpReply::encodeResponse(HttpBufferPtr httpBuffer,
                                 std::vector<boost::asio::const_buffer>& buffers)
  {
    LOG_DEBUG("HttpReply::encodeResponse()");
    std::vector<boost::asio::const_buffer> result;

    if (!m_gzipBusy) {
      initGzip();
    }

    unsigned int i = 0;

    for (auto& buffer : buffers) {

      m_gzip.avail_in = boost::asio::buffer_size(buffer);
      m_gzip.next_in = (unsigned char*) boost::asio::detail::buffer_cast_helper(
                         buffer);

      unsigned char out[16 * 1024];

      do {
        m_gzip.next_out = out;
        m_gzip.avail_out = sizeof(out);

        int res = 0;
        res = deflate(&m_gzip,
                      (i == buffers.size() - 1 && m_request->completed()) ?
                      Z_FINISH : Z_NO_FLUSH);
        assert(res != Z_STREAM_ERROR);

        unsigned have = sizeof(out) - m_gzip.avail_out;
        m_contentLength += have;

        if (have) {
          result.push_back(buf(httpBuffer, std::string((char*)out, have)));
        }
      } while (m_gzip.avail_out == 0);

      ++i;

    }

    if (m_request->completed()) {
      deflateEnd(&m_gzip);
      m_gzipBusy = false;
    }

    buffers.clear();
    buffers.insert(buffers.begin(), result.begin(), result.end());

    return true;
  }
#endif // ENABLE_GZIP

  void HttpReply::buildHeaders(HttpBufferPtr httpBuffer)
  {
    LOG_DEBUG("HttpReply::buildHeaders()");
    std::stringstream stream;
    stream << "HTTP/" << m_request->httpVersion() << " ";
    stringValue(m_status, stream);

    if (canChunkResponse()) {
      m_replyHeaders->addHeader("Transfer-Encoding", "chunked");
    } else {
      m_replyHeaders->addHeader("Content-Length", m_contentLength);
    }

    if (canEncodeResponse()) {
      LOG_DEBUG("Content-Encoding: gzip");
      m_replyHeaders->addHeader("Content-Encoding", "gzip");
    }

    if (getFlag(Flag::Closing)) {
      m_replyHeaders->addHeader("Connection", "close");
    } else {
      m_replyHeaders->addHeader("Connection", "keep-alive");
    }

    m_replyHeaders->serialize(stream);
    const std::string& resp = stream.str();
    httpBuffer->m_buffers[0] = buf(httpBuffer, std::string(resp));
  }

  void HttpReply::buildErrorResponse(Code error, std::stringstream& stream,
                                     bool closeConnection)
  {
    LOG_DEBUG("HttpReply::buildErrorResponse()");
    bool found = false;
    size_t bytes = 0;

    const ErrorPageMap& map = m_request->hasMatchingSite() ?
                              m_request->matchingSite()->m_errorPages :
                              m_request->session()->config()->m_errorPages;

    if (map.count(std::to_string((int)error)) > 0 ) {
      std::string url = map.at(std::to_string((int)error));

      if (m_request->cacheHandler()->read(url, stream, bytes) == Code::Ok) {
        found = true;
        setMimeType(url);
      }
    }

    if (!found) {
      stream << "<html> <body><h1>";
      stringValue(error, stream);
      stream << " </h1></body></html>";
      bytes = stream.str().length();
    }

    setFlag(Flag::Closing, closeConnection);

    m_request->setCompleted(true);
    m_status = error;
    m_contentLength = bytes;

    post(stream);

  }
#ifdef ENABLE_GZIP
  void HttpReply::initGzip()
  {
    LOG_DEBUG("HttpReply::initGzip()");
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
#endif // ENABLE_GZIP

  boost::asio::const_buffer HttpReply::buf(HttpBufferPtr buffer,
      const std::string& s)
  {
    buffer->m_bufs.push_back(s);
    return boost::asio::buffer(buffer->m_bufs.back());
  }

  boost::asio::const_buffer HttpReply::buf(HttpBufferPtr buffer, char* buf,
      size_t s)
  {
    buffer->m_bufs2.push_back(buf);
    return boost::asio::buffer(buffer->m_bufs2.back(), s);
  }

  void HttpReply::setMimeType(const std::string& filename)
  {
    m_replyHeaders->addHeader("Content-Type", FileUtils::mimeType(filename));

    std::string eTag = m_request->cacheHandler()->eTag(filename);

    if (eTag.length() > 0) {
      m_replyHeaders->addHeader("ETag", eTag);
    }

    std::string lm = m_request->cacheHandler()->lastModified(filename);

    if (lm.length() > 0) {
      m_replyHeaders->addHeader("Last-Modified", lm);
    }

    if (!m_request->hasMatchingSite()) {
      setFlag(Flag::GzipEncoding, false);
      return;
    }

    auto site = m_request->matchingSite();

    if (site->m_gzip.count("all") == 0
        && site->m_gzip.count(FileUtils::extension(filename)) == 0) {
      setFlag(Flag::GzipEncoding, false);
    }

  }

  bool HttpReply::canChunkResponse() const
  {
    return getFlag(Flag::ChunkedEncoding)
           && m_request->method() != Method::HEAD
           && m_status != Code::PartialContent;
  }

  bool HttpReply::canEncodeResponse() const
  {
#ifdef ENABLE_GZIP
    return getFlag(Flag::GzipEncoding) && m_request->method() != Method::HEAD;
#else
    return false;
#endif // ENABLE_GZIP
  }

}

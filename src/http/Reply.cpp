#include "Reply.h"

#include "Request.h"
#include "Headers.h"
#include "utils/FileUtils.h"
#include "utils/Logger.h"
#include "cache/generic_cache.h"
#include "common/Params.h"
#include "common/Buffer.h"
#include "gateway/GatewayHandler.h"
#include "core/SessionConnection.h"

#include <boost/algorithm/string/replace.hpp>
#include <algorithm>
#include <regex>
#include <functional>

namespace Interceptor::Http {

  Reply::Reply(HttpRequestPtr request)
    : m_request(request),
      m_replyHeaders(nullptr),
      m_gateway(nullptr),
      m_status(Code::Ok),
      m_contentLength(0),
      m_gzipBusy(false)
  {
  }

  Reply::~Reply()
  {
    LOG_DEBUG("Reply::~Reply()");
  }

  void Reply::process()
  {
    LOG_DEBUG("Reply::process()");
    std::stringstream stream;
    m_httpBuffer = std::make_shared<Buffer>();

    if (m_replyHeaders) {
      m_replyHeaders.reset();
    }

    m_replyHeaders = std::make_unique<Headers>();
    m_replyHeaders->addGeneralHeaders();

    if ( !m_request->headersReceived() ) {
      std::stringstream stream;
      buildErrorResponse(Code::BadRequest, true);
      return;
    }

    Code status =  m_request->parse();

    if (status != Code::Ok) {
      if (m_request->m_headers) {
        m_replyHeaders->fillFrom(m_request->m_headers.get());
      }

      buildErrorResponse(status, true);
      return;
    }

    setFlag(Flag::ChunkedEncoding, m_request->supportsChunking());

#ifdef ENABLE_GZIP
    setFlag(Flag::GzipEncoding, m_request->supportsCompression());
#endif // ENABLE_GZIP

    if (!m_request->hasMatchingSite()) {
      buildErrorResponse(Code::NotFound, true);
      return;
    }

    const SiteConfig* site = m_request->matchingSite();

    if (site->m_backend.length() > 0) {
      m_gateway = std::make_unique<GatewayHandler>(site, m_request,
                  m_request->params()->m_pool);
      std::weak_ptr<Reply> wp {shared_from_this()};
      m_gateway->route(std::bind([wp] (Http::Code code, std::stringstream * stream) {
        auto ptr = wp.lock();

        if (ptr) {
          ptr->handleGatewayReply(code, stream);
        }
      }, std::placeholders::_1, std::placeholders::_2));
      return ;
    }

    switch (m_request->method()) {
      case Method::GET:
      case Method::HEAD:
        handleRetrievalRequest(m_request->method(), site);
        break;

      case Method::POST:
        break;

      default:
        break;
    }
  }

  void Reply::handleGatewayReply(Code code, std::stringstream* stream)
  {
    LOG_DEBUG("Reply::handleGatewayReply()");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_httpBuffer = std::make_shared<Buffer>();
    LOG_DEBUG("Received from gateway ");
    buildErrorResponse(code, false);
  }

  void Reply::setFlag(Flag flag, bool value)
  {
    m_flags.set(flag, value);
  }

  bool Reply::getFlag(Flag flag) const
  {
    return m_flags.test(flag);
  }

  void Reply::declineRequest(Code error)
  {
    Code ret;
    LOG_DEBUG("Reply::declineRequest()");
    m_httpBuffer = std::make_shared<Buffer>();
    m_replyHeaders = std::make_unique<Headers>();

    if (m_request->headersReceived())  {
      if (!m_request->parsed()) {
        ret = m_request->parse();

        if (ret != Code::Ok) {
          error = ret;
        }
      }
    } else {
      m_httpBuffer->m_flags |= Buffer::State::InvalidRequest;
    }

    buildErrorResponse(error, true);
  }

  void Reply::handleRetrievalRequest(Method method, const SiteConfig* site)
  {
    LOG_DEBUG("Reply::handleRetrievalRequest()");

    Code ret;

    if ( (ret = hasSpecialLocationCode(site)) != Code::Ok) {
      buildErrorResponse(ret, true);
      return;
    }

    requestFileContents(method, site);
  }

  bool Reply::requestFileContents(Method method, const SiteConfig* site)
  {
    std::stringstream stream;
    LOG_DEBUG("Reply::requestFileContents()");
    std::string page;
    bool found = false;
    size_t bytes = 0;

    if ( m_request->index() == ""
         || m_request->index() == "/"
         || m_request->index().at(m_request->index().length() - 1) == '/'
       ) {
      // This request does not contain a filename, we will use a page from try-file
      std::vector<std::string> tryFiles = site->m_tryFiles;

      for (const auto& index : tryFiles) {
        page = site->m_docroot + m_request->index() + "/" + index;

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
      buildErrorResponse(Code::NotFound);
      return false;
    }

    Code ret = Code::Ok;

    // page found
    if ( method == Method::GET) {
      boost::algorithm::replace_all(page, "//", "/");

      if (m_request->partialRequest()) {
        ret = requestPartialFileContents(page, stream, bytes);
      } else {
        if (!m_request->cacheHandler()->size(page, bytes)) {
          buildErrorResponse(Code::InternalServerError);
          return false;
        }

        if (bytes > MAX_CHUNK_SIZE) {
          // File is too big to be sent at once, we will send it in multiple times to
          // avoid consuming to much memory
          setMimeType(page);
          m_contentLength = bytes;
          return requestLargeFileContents(page, 0, bytes, bytes);
        } else {
          ret = requestFileContents(page, stream, bytes);
        }
      }
    } else if (method == Method::HEAD) {
      if (!m_request->cacheHandler()->size(page, bytes)) {
        ret = Code::NotFound;
      } else {
        setMimeType(page);
        m_contentLength = bytes;
        m_request->setCompleted(true);
        post(stream);
      }

    }

    if (ret != Code::Ok) {
      buildErrorResponse(ret);
      return false;
    }

    return true;
  }

  Code Reply::requestFileContents(const std::string& page,
                                  std::stringstream& stream, size_t bytes)
  {
    Code ret = m_request->cacheHandler()->read(page, stream, bytes);

    if (ret == Code::Ok) {
      setMimeType(page);
      m_request->setCompleted(true);
      m_contentLength = bytes;
      post(stream);
    }

    return ret;
  }

  Code Reply::requestPartialFileContents(const std::string& page,
                                         std::stringstream& stream, size_t& bytes)
  {
    LOG_DEBUG("Reply::requestPartialFileContents()");
    std::tuple<int64_t, int64_t> range;
    Code ret;

    if ((ret = m_request->getRangeRequest(range)) != Code::Ok) {
      return ret;
    }

    int64_t from = std::get<0>(range);
    int64_t to = std::get<1>(range);
    size_t total = 0;
    ret = FileUtils::calculateBounds(page, from, to);

    if (ret != Code::Ok) {
      return ret;
    }

    if (!FileUtils::fileSize(page, total)) {
      return Code::NotFound;
    }

    bytes = to - from + 1;
    m_contentLength = bytes;

    m_replyHeaders->addHeader("Content-Range",
                              "bytes " + std::to_string(from) + "-"
                              + std::to_string(to) + "/" + std::to_string(total));
    m_status = Code::PartialContent;
    setMimeType(page);

    if (to - from > MAX_CHUNK_SIZE) {
      // File is too big to be sent at once, we will send it in multiple times to
      // avoid consuming to much memory
      return requestLargeFileContents(page, from, to,
                                      total) ? Code::Ok : Code::BadRequest;
    } else {
      ret = FileUtils::readFile(page,	from, to , stream,
                                bytes); //TODO take it from cache

      if (ret == Code::Ok) {
        m_request->setCompleted(true);
        post(stream);
      }
    }

    return ret;
  }

  bool Reply::requestLargeFileContents(const std::string& page, size_t from,
                                       size_t limit,
                                       size_t totalBytes)
  {
    LOG_DEBUG("Reply::requestLargeFileContents()");
    //needed to be sure that previous call is completed
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t bytes;
    size_t to = std::min(limit, std::min((size_t) from + MAX_CHUNK_SIZE,
                                         totalBytes - 1));
    setFlag(LargeFileRequest, true);

    m_httpBuffer = std::make_shared<Buffer>();
    std::stringstream stream;

    if (FileUtils::readFile(page, from, to, stream, bytes) == Code::Ok) {
      if (to == limit - 1) {
        m_request->setCompleted(true);
        post(stream);
        return true;
      } else {
        from = to + 1;
        m_httpBuffer->m_nextCall = std::bind(&Reply::requestLargeFileContents,
                                             shared_from_this(), page, from , limit, totalBytes);
        m_httpBuffer->m_flags |= Buffer::HasMore;
        post(stream);
      }

    } else {
      return false;
    }

    return true;
  }

  void Reply::post(const std::stringstream& stream)
  {
    LOG_DEBUG("Reply::post()");
#ifdef DUMP_NETWORK
    LOG_DEBUG("Reply: " << "\n" << stream.str());
#endif //DUMP_NETWORK
    std::vector<boost::asio::const_buffer> buffers;

    if (!(m_httpBuffer->flags() & Buffer::InvalidRequest)) {
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
        m_httpBuffer->m_flags |= Buffer::Closing;
      }
    }

    auto connection = m_request->connection();

    if (connection) {
      connection->postReply(m_httpBuffer);
    }

    m_httpBuffer.reset();
  }

  bool Reply::chunkResponse(BufferPtr httpBuffer,
                            std::vector<boost::asio::const_buffer>& buffers)
  {
    LOG_DEBUG("Reply::chunkResponse()");
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
  bool Reply::encodeResponse(BufferPtr httpBuffer,
                             std::vector<boost::asio::const_buffer>& buffers)
  {
    LOG_DEBUG("Reply::encodeResponse()");
    std::vector<boost::asio::const_buffer> result;

    if (!m_gzipBusy) {
      initGzip();
    }

    unsigned int i = 0;

    for (auto& buffer : buffers) {

      m_gzip.avail_in = boost::asio::buffer_size(buffer);
      m_gzip.next_in = (unsigned char*) boost::asio::detail::buffer_cast_helper(
                         buffer);

      char out[16 * 1024];

      do {
        m_gzip.next_out = (unsigned char*)out;
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

  void Reply::buildHeaders(BufferPtr httpBuffer)
  {
    LOG_DEBUG("Reply::buildHeaders()");
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

  void Reply::buildErrorResponse(Code error, bool closeConnection)
  {
    LOG_DEBUG("Reply::buildErrorResponse()");
    std::stringstream stream;
    bool found = false;
    size_t bytes = 0;

    const ErrorPageMap& map = m_request->hasMatchingSite() ?
                              m_request->matchingSite()->m_errorPages :
                              m_request->params()->config()->m_errorPages;

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
  void Reply::initGzip()
  {
    LOG_DEBUG("Reply::initGzip()");
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

  boost::asio::const_buffer Reply::buf(BufferPtr buffer,
                                       const std::string& s)
  {
    buffer->m_bufs.push_back(s);
    return boost::asio::buffer(buffer->m_bufs.back());
  }

  boost::asio::const_buffer Reply::buf(BufferPtr buffer, char* buf,
                                       size_t s)
  {
    buffer->m_bufs2.push_back(buf);
    return boost::asio::buffer(buffer->m_bufs2.back(), s);
  }

  void Reply::setMimeType(const std::string& filename)
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

  bool Reply::canChunkResponse() const
  {
    return getFlag(Flag::ChunkedEncoding)
           && m_request->method() != Method::HEAD
           && m_status != Code::PartialContent;
  }

  bool Reply::canEncodeResponse() const
  {
#ifdef ENABLE_GZIP
    return getFlag(Flag::GzipEncoding) && m_request->method() != Method::HEAD
           && m_status != Code::PartialContent;
#else
    return false;
#endif // ENABLE_GZIP
  }

  Code Reply::hasSpecialLocationCode(const SiteConfig* site) const
  {
    std::string idx = m_request->index();

    for (const auto& kv : site->m_locations) {
      std::regex reg(kv.first, std::regex_constants::ECMAScript);

      if (std::regex_search(idx, reg) > 0) {
        return (Code)kv.second;
      }
    }

    return Code::Ok;
  }
}
#include "CommonReply.h"

#include "global.h"
#include "HttpException.h"
#include "Request.h"
#include "utils/Logger.h"
#include "utils/FileUtils.h"
#include "utils/StringUtils.h"
#include "common/Buffer.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace Interceptor::Http {

  CommonReply::CommonReply(HttpRequestPtr request, const SiteConfig* config)
    : m_request(request),
      m_replyHeaders(std::make_unique<Headers>()),
      m_httpBuffer(std::make_shared<Buffer>()),
      m_encoder(std::make_unique<Encoder>()),
      m_config(config),
      m_contentLength(0),
      m_status(StatusCode::Ok)
  {
  }

  StatusCode CommonReply::getLocationCode(const SiteConfig* site) const
  {
    std::string idx = m_request->index();

    for (const auto& kv : site->m_locations) {
      if (StringUtils::regexMatch(kv.first, idx)) {
        return (StatusCode)kv.second;
      }
    }

    return StatusCode::Ok;
  }

  void CommonReply::setFlag(Flag flag, bool value)
  {
    m_flags.set(flag, value);
  }

  bool CommonReply::getFlag(Flag flag) const
  {
    return m_flags.test(flag);
  }


  void CommonReply::requestFileContents(const std::string& page,
                                        std::stringstream& stream, size_t& bytes)
  {
    StatusCode ret = m_request->cacheHandler()->read(page, stream, bytes);

    if (ret == StatusCode::Ok) {
      setHeadersFor(page);
      m_request->setCompleted(true);
      m_contentLength = bytes;
    } else {
      throw HttpException(ret, false);
    }
  }

  void CommonReply::requestPartialFileContents(const std::string& page,
      std::stringstream& stream, size_t& bytes)
  {
    LOG_DEBUG("CommonReply::requestPartialFileContents()");
    std::tuple<int64_t, int64_t> range;
    StatusCode ret;

    range = m_request->getRangeRequest();

    int64_t from = std::get<0>(range);
    int64_t to = std::get<1>(range);
    size_t total = 0;
    ret = FileUtils::calculateBounds(page, from, to);

    if (ret != StatusCode::Ok) {
      throw HttpException(ret, true);
    }

    if (!FileUtils::fileSize(page, total)) {
      throw HttpException(StatusCode::NotFound, false);
    }

    bytes = to - from + 1;
    m_contentLength = bytes;

    m_replyHeaders->addHeader("Content-Range",
                              "bytes " + std::to_string(from) + "-"
                              + std::to_string(to) + "/" + std::to_string(total));
    m_status = StatusCode::PartialContent;
    setHeadersFor(page);

    if (to - from > MAX_CHUNK_SIZE) {
      // File is too big to be sent at once, we will send it in multiple times to
      // avoid consuming to much memory
      requestLargeFileContents(page, stream, from, to, total);
    } else {
      ret = FileUtils::readFile(page,	from, to, stream,
                                bytes); //TODO take it from cache

      if (ret == StatusCode::Ok) {
        m_request->setCompleted(true);
      } else {
        throw HttpException(ret, true);
      }
    }
  }

  BufferPtr CommonReply::requestFileChunk(const std::string& page, size_t from,
                                          size_t limit, size_t& bytes)
  {
    std::stringstream stream;

    requestLargeFileContents(page, stream, from, limit, bytes);
    serialize(stream);

    return m_httpBuffer;
  }


  void CommonReply::requestLargeFileContents(const std::string& page,
      std::stringstream& stream, size_t from,
      size_t limit,
      size_t totalBytes)
  {
    LOG_DEBUG("CommonReply::requestLargeFileContents()");
    //needed to be sure that previous call is completed
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t bytes;
    size_t to = std::min(limit, std::min((size_t) from + MAX_CHUNK_SIZE,
                                         totalBytes - 1));
    setFlag(LargeFileRequest, true);

    m_httpBuffer = std::make_shared<Buffer>();

    StatusCode ret;

    if ((ret = FileUtils::readFile(page, from, to, stream,
                                   bytes)) == StatusCode::Ok) {
      if (to == limit - 1) {
        m_request->setCompleted(true);
      } else {
        from = to + 1;
        m_httpBuffer->m_nextCall = std::bind(&CommonReply::requestFileChunk,
                                             shared_from_this(), page, from, limit, totalBytes);
        m_httpBuffer->m_flags |= Buffer::HasMore;
      }

    } else {
      throw HttpException(ret, true);
    }
  }

  std::string CommonReply::requestedPath(HttpRequestPtr request,
                                         const SiteConfig* config)
  {
    LOG_DEBUG("CommonReply::RequestedPath()");
    std::string page;

    if ( CommonReply::isRequestingRoot(request, config)) {
      page = CommonReply::getRootFile(request, config);
    } else {
      // This request contains the filename, hence we should
      // not try a filename from the list of try-files
      page = config->m_docroot + request->index();

      if (!FileUtils::exists(page)) {
        throw HttpException(StatusCode::NotFound);
      }
    }

    // page found
    boost::algorithm::replace_all(page, "///", "/");
    boost::algorithm::replace_all(page, "//", "/");
    return page;

  }

  std::string CommonReply::requestedPath() const
  {
    return CommonReply::requestedPath(m_request, m_config);
  }

  bool CommonReply::isRequestingRoot(HttpRequestPtr request,
                                     const SiteConfig* config)
  {
    try {
      boost::filesystem::path p(config->m_docroot + "/" + request->index());
      return boost::filesystem::is_directory(p);
    } catch (const boost::filesystem::filesystem_error& e) {
      return false;
    }
  }

  std::string CommonReply::getRootFile(HttpRequestPtr request,
                                       const SiteConfig* config)
  {
    std::string page;
    bool found = false;
    std::vector<std::string> tryFiles = config->m_tryFiles;

    for (const auto& index : tryFiles) {
      page = config->m_docroot + request->index() + "/" + index;

      if (FileUtils::exists(page)) {
        found = true;
        break;
      }
    }

    if (!found) {
      page = config->m_docroot + request->index();

      // Are we serving an index ?
      if (!FileUtils::exists(config->m_docroot + request->index())) {
        throw HttpException(StatusCode::NotFound);
      }
    }

    return page;
  }

  void CommonReply::setHeadersFor(const std::string& filename)
  {

    m_replyHeaders->setHeadersFor(filename, m_request->cacheHandler());

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

  bool CommonReply::canChunkResponse() const
  {
    return getFlag(Flag::ChunkedEncoding)
           && m_request->method() != Method::HEAD
           && m_status != StatusCode::PartialContent;
  }

  bool CommonReply::canEncodeResponse() const
  {
#ifdef ENABLE_GZIP
    return getFlag(Flag::GzipEncoding) && m_request->method() != Method::HEAD
           && m_status != StatusCode::PartialContent;
#else
    return false;
#endif // ENABLE_GZIP
  }

  bool CommonReply::shouldCloseConnection() const
  {
    return m_request->closeConnection() || getFlag(Closing);
  }

  void CommonReply::buildStatusLine(std::stringstream& stream)
  {
    stream << "HTTP/" << m_request->httpVersion() << " ";
    serializeHttpCode(m_status, stream);
  }

  void CommonReply::buildHeaders(BufferPtr httpBuffer)
  {
    LOG_DEBUG("CommonReply::buildHeaders()");
    std::stringstream stream;

    buildStatusLine(stream);

    if (canChunkResponse()) {
      m_replyHeaders->addHeader("Transfer-Encoding", "chunked");
    } else {
      m_replyHeaders->addHeader("Content-Length", m_contentLength);
    }

    if (canEncodeResponse()) {
      LOG_DEBUG("Content-Encoding: gzip");
      m_replyHeaders->addHeader("Content-Encoding", "gzip");
    }

    if (shouldCloseConnection()) {
      m_replyHeaders->addHeader("Connection", "close");
      httpBuffer->m_flags |= Buffer::Closing;
    } else {
      m_replyHeaders->addHeader("Connection", "keep-alive");
    }

    if (m_config) {
      time_t cacheTime = m_config->getCacheTime(m_request->index());

      if (cacheTime > 0) {
        m_replyHeaders->addHeader("Cache-Control",
                                  "public, max-age=" + std::to_string(cacheTime));
      } else {
        m_replyHeaders->addHeader("Cache-Control", "no-cache");
      }
    }

    m_replyHeaders->serialize(stream);
    const std::string& resp = stream.str();

    if (httpBuffer->m_buffers.size() == 0) {
      httpBuffer->m_buffers.push_back(httpBuffer->buf(std::string(resp)));
    } else {
      httpBuffer->m_buffers[0] = httpBuffer->buf(std::string(resp));
    }
  }

}

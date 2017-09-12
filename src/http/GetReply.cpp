#include "GetReply.h"

#include "Encoder.h"
#include "HttpException.h"
#include "utils/Logger.h"
#include "utils/FileUtils.h"

#include "common/Buffer.h"

#include <boost/asio.hpp>


namespace Interceptor::Http {

  GetReply::GetReply(HttpRequestPtr request, const SiteConfig* config)
    : CommonReply(request, config)
  {
    m_replyHeaders->addGeneralHeaders();
    setFlag(Flag::ChunkedEncoding, m_request->supportsChunking());
#ifdef ENABLE_GZIP
    setFlag(Flag::GzipEncoding, m_request->supportsCompression());
#endif // ENABLE_GZIP
  }

  BufferPtr GetReply::buildReply()
  {
    LOG_DEBUG("GetReply::handleRetrievalRequest()");

    Code ret;

    if ( (ret = getLocationCode(m_config)) != Code::Ok) {
      throw HttpException(ret, true);
    }

    processRequest();
    return m_httpBuffer;
  }


  void GetReply::processRequest()
  {
    LOG_DEBUG("GetReply::requestFileContents()");

    if (m_request->hasIfModifiedSince()) {
      if (requestIfMofidiedSince()) {
        return;
      }
    }

    requestFileContents();
  }

  void GetReply::requestFileContents()
  {
    std::stringstream stream;
    size_t bytes = 0;
    std::string page;

    page = requestedPath();

    if (m_request->partialRequest()) {
      requestPartialFileContents(page, stream, bytes);
    } else {
      if (!m_request->cacheHandler()->size(page, bytes)) {
        throw HttpException(Code::InternalServerError, true);
      }

      if (bytes > MAX_CHUNK_SIZE) {
        // File is too big to be sent at once, we will send it in multiple times to
        // avoid consuming to much memory
        setHeadersFor(page);
        m_contentLength = bytes;

        requestLargeFileContents(page, stream, 0, bytes, bytes);

      } else {
        CommonReply::requestFileContents(page, stream, bytes);
      }
    }

    serialize(stream);
  }

  bool GetReply::requestIfMofidiedSince()
  {
    std::time_t requestedTime = m_request->ifModifiedSince();

    if (requestedTime <= 0) {
      throw HttpException(Code::BadRequest, true);
    }

    std::time_t lastModified = FileUtils::lastModified(requestedPath());

    if (lastModified > requestedTime) {
      return false;
    }

    std::stringstream stream;
    m_status = Code::NotModified;
    buildStatusLine(stream);
    const std::string* header =  m_request->getHeader("If-Modified-Since");
    m_replyHeaders->addHeader("Date", *header);

    m_replyHeaders->serialize(stream);

    m_httpBuffer->m_buffers.push_back(m_httpBuffer->buf(stream.str()));

    return true;
  }

  void GetReply::serialize(std::stringstream& stream)
  {
    LOG_DEBUG("GetReply::serialize()");
    LOG_NETWORK("Posting Stream:", stream.str());
    std::vector<boost::asio::const_buffer> buffers;

    if (!(m_httpBuffer->flags() & Buffer::InvalidRequest)) {
      std::vector<boost::asio::const_buffer> buffers;

      if (!getFlag(HeadersSent)) {
        m_httpBuffer->m_buffers.push_back({}); //empty slot for headers
      }

      buffers.push_back(m_httpBuffer->buf(std::string(stream.str())));

#ifdef ENABLE_GZIP

      if (canEncodeResponse()) {
        m_encoder->encode(m_httpBuffer, buffers, m_request->completed(),
                          m_contentLength);
      }

#endif // ENABLE_GZIP

      // We chunk only in the case that no header has been sent or if it's the last frame
      if (canChunkResponse()) {
        m_encoder->chunk(m_httpBuffer, buffers, m_request->completed()
                         || !getFlag(LargeFileRequest));
      }

      if (!getFlag(HeadersSent)) {
        buildHeaders(m_httpBuffer);
        LOG_INFO(m_request->queryString() << " " << (int) m_status);
        setFlag(HeadersSent, true);
      }

      m_httpBuffer->m_buffers.insert(m_httpBuffer->m_buffers.end(), buffers.begin(),
                                     buffers.end());

    }
  }
}

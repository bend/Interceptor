#include "GetReply.h"

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

    requestFileContents();
    return m_httpBuffer;
  }


  void GetReply::requestFileContents()
  {
    std::stringstream stream;
    LOG_DEBUG("GetReply::requestFileContents()");
    std::string page;
    size_t bytes = 0;

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

        if (!requestLargeFileContents(page, stream, 0, bytes, bytes)) {
          throw HttpException(Code::InternalServerError, true);
        }

      } else {
        CommonReply::requestFileContents(page, stream, bytes);
      }
    }

    serialize(stream);
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

    }
  }
}

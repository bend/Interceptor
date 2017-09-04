#ifndef COMMON_REPLY_H__
#define COMMON_REPLY_H__

#include "common/Defs.h"
#include "common/Params.h"

#include "Headers.h"

#include <sstream>
#include <bitset>
#include <mutex>

#include <boost/asio/buffer.hpp>
#include <zlib.h>


namespace Interceptor::Http {

  class CommonReply : public std::enable_shared_from_this<CommonReply> {
  public:
    enum Flag {
      Closing,
      GzipEncoding,
      ChunkedEncoding,
      HeadersSent,
      LargeFileRequest
    };

  public:
    virtual BufferPtr buildReply() = 0;

    void setFlag(Flag flag, bool value);

    bool getFlag(Flag flag) const;

  protected:
    CommonReply(HttpRequestPtr request, const SiteConfig* config);

    Code getLocationCode(const SiteConfig* config) const;

    void requestPartialFileContents(const std::string& page,
                                    std::stringstream& stream, size_t& bytes);

    BufferPtr requestFileChunk(const std::string& page, size_t from, size_t limit,
                               size_t& bytes);

    bool requestLargeFileContents(const std::string& page,
                                  std::stringstream& stream, size_t from,
                                  size_t limit,
                                  size_t totalBytes);
    void requestFileContents(const std::string& page,
                             std::stringstream& stream, size_t bytes);
    bool isRequestingRoot() const;

    std::string requestedPath() const;

    std::string getRootFile() const;

    void setHeadersFor(const std::string& filename);

    void buildHeaders(BufferPtr httpBuffer);

    bool chunkResponse(BufferPtr httpBuffer,
                       std::vector<boost::asio::const_buffer>& buffers);

    virtual void serialize(std::stringstream& stream) = 0;
#ifdef ENABLE_GZIP
    bool encodeResponse(BufferPtr httpBuffer,
                        std::vector<boost::asio::const_buffer>& buffers);
    void initGzip();
#endif // ENABLE_GZIP

    bool canChunkResponse() const;
    bool canEncodeResponse() const;



  protected:
    HttpRequestPtr m_request;
    HttpHeaderUPtr m_replyHeaders;
    BufferPtr m_httpBuffer;
    const SiteConfig* m_config;

    size_t m_contentLength;
    std::bitset<5> m_flags;

    z_stream m_gzip;
    bool m_gzipBusy;
    Code m_status;

    std::mutex m_mutex;
  };

}

#endif // COMMON_REPLY_H__

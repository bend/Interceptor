#ifndef COMMON_REPLY_H__
#define COMMON_REPLY_H__

#include "common/Defs.h"
#include "common/Params.h"

#include "Headers.h"
#include "Encoder.h"

#include <sstream>
#include <bitset>
#include <mutex>

#include <boost/asio/buffer.hpp>

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

    static std::string requestedPath(HttpRequestPtr request,
                                     const SiteConfig* config);

    static bool isRequestingRoot(HttpRequestPtr request, const SiteConfig* config);

    static std::string getRootFile(HttpRequestPtr request,
                                   const SiteConfig* config);

  protected:
    CommonReply(HttpRequestPtr request, const SiteConfig* config);

    Code getLocationCode(const SiteConfig* config) const;

    void requestPartialFileContents(const std::string& page,
                                    std::stringstream& stream, size_t& bytes);

    BufferPtr requestFileChunk(const std::string& page, size_t from, size_t limit,
                               size_t& bytes);

    void requestLargeFileContents(const std::string& page,
                                  std::stringstream& stream, size_t from,
                                  size_t limit,
                                  size_t totalBytes);
    void requestFileContents(const std::string& page,
                             std::stringstream& stream, size_t bytes);

    std::string requestedPath() const;

    void setHeadersFor(const std::string& filename);

    void buildStatusLine(std::stringstream& strea);

    void buildHeaders(BufferPtr httpBuffer);

    virtual void serialize(std::stringstream& stream) = 0;

    bool canChunkResponse() const;
    bool canEncodeResponse() const;
    bool shouldCloseConnection() const ;

  protected:
    HttpRequestPtr m_request;
    HttpHeaderUPtr m_replyHeaders;
    BufferPtr m_httpBuffer;
    std::unique_ptr<Encoder> m_encoder;

    const SiteConfig* m_config;

    size_t m_contentLength;
    std::bitset<5> m_flags;

    Code m_status;

    std::mutex m_mutex;

  };

}

#endif // COMMON_REPLY_H__

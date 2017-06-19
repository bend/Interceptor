#ifndef HTTP_REPLY_H__
#define HTTP_REPLY_H__

#include "common/Defs.h"
#include "vars.h"
#include "Http.h"
#include "core/Config.h"

#include <boost/asio.hpp>
#include <bitset>

#include <zlib.h>
#include <boost/thread/mutex.hpp>

namespace Http {
  class HttpHeaders;

  class HttpRequest;

  enum class Code : short;
    enum class Method : char;

    class HttpReply : public std::enable_shared_from_this<HttpReply> {

  public:
    enum Flag {
      Closing,
      GzipEncoding,
      ChunkedEncoding,
      HeadersSent,
      LargeFileRequest
    };

    HttpReply(HttpRequestPtr request);
    ~HttpReply();

    void process();

    void setFlag(Flag flag, bool value);

    bool getFlag(Flag flag) const;

  private:
    void handleRetrievalRequest(Method method);

    void post(const std::stringstream& stream);
    void buildErrorResponse(Code error, bool closeConnection = false);

    bool chunkResponse(HttpBufferPtr httpBuffer,
                       std::vector<boost::asio::const_buffer>& buffers);
#ifdef ENABLE_GZIP
    bool encodeResponse(HttpBufferPtr httpBuffer,
                        std::vector<boost::asio::const_buffer>& buffers);
#endif // ENABLE_GZIP

    void buildHeaders(HttpBufferPtr httpBuffer);

#ifdef ENABLE_GZIP
    void initGzip();
#endif // ENABLE_GZIP

    void setMimeType(const std::string& filename);
    bool requestFileContents(Method method, const SiteConfig* site);
    Code requestPartialFileContents(const std::string& page,
                                    std::stringstream& stream, size_t& bytes);
    bool requestLargeFileContents(const std::string& page, size_t from,
                                  size_t limit,
                                  size_t totalBytes);
    Code requestFileContents(const std::string& page, std::stringstream& stream,
                             size_t bytes);

    boost::asio::const_buffer buf(HttpBufferPtr buffer, const std::string& s);
    boost::asio::const_buffer buf(HttpBufferPtr buffer, char* buf, size_t s);

    bool canChunkResponse() const;
    bool canEncodeResponse() const;

    Code hasSpecialLocationCode(const SiteConfig* site) const;

  private:
    HttpRequestPtr m_request;
    HttpHeaders* m_replyHeaders;
    Code m_status;

    std::bitset<5> m_flags;
    size_t m_contentLength;

    z_stream m_gzip;
    bool m_gzipBusy;

    HttpBufferPtr m_httpBuffer;

    boost::mutex m_mutex;
  };

}
#endif // HTTP_REPLY_H__

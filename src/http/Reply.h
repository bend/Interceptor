#ifndef HTTP_REPLY_H__
#define HTTP_REPLY_H__

#include "common/Defs.h"
#include "vars.h"
#include "Http.h"
#include "core/Config.h"

#include <boost/asio.hpp>

#include <bitset>
#include <mutex>

#include <zlib.h>


namespace Interceptor {

  class GatewayHandler;

  namespace Http {
    class Request;

    enum class Code : short;
      enum class Method : char;

      class Reply : public std::enable_shared_from_this<Reply> {

    public:
      enum Flag {
        Closing,
        GzipEncoding,
        ChunkedEncoding,
        HeadersSent,
        LargeFileRequest
      };

      Reply(HttpRequestPtr request);
      ~Reply();

      void process();

      void setFlag(Flag flag, bool value);

      bool getFlag(Flag flag) const;

      void declineRequest(Code error);

    private:
      void handleRetrievalRequest(Method method, const SiteConfig* site);
      void handleGatewayReply(Code code, std::stringstream*  stream);

      void post(const std::stringstream& stream);
      void postBackendReply(const std::stringstream& stream);
      void buildErrorResponse(Code error, bool closeConnection = false);

      bool chunkResponse(BufferPtr httpBuffer,
                         std::vector<boost::asio::const_buffer>& buffers);
#ifdef ENABLE_GZIP
      bool encodeResponse(BufferPtr httpBuffer,
                          std::vector<boost::asio::const_buffer>& buffers);
#endif // ENABLE_GZIP

      void buildHeaders(BufferPtr httpBuffer);

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

      boost::asio::const_buffer buf(BufferPtr buffer, const std::string& s);
      boost::asio::const_buffer buf(BufferPtr buffer, char* buf, size_t s);

      bool canChunkResponse() const;
      bool canEncodeResponse() const;

      Code hasSpecialLocationCode(const SiteConfig* site) const;

    private:
      typedef std::unique_ptr<GatewayHandler> GatewayHandlerUPtr;
      HttpRequestPtr m_request;
      HttpHeaderUPtr m_replyHeaders;
      GatewayHandlerUPtr m_gateway;
      Code m_status;

      std::bitset<5> m_flags;
      size_t m_contentLength;

      z_stream m_gzip;
      bool m_gzipBusy;

      BufferPtr m_httpBuffer;

      std::mutex m_mutex;
    };

  }

}
#endif // HTTP_REPLY_H__

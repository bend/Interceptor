#ifndef HTTP_REPLY_H__
#define HTTP_REPLY_H__

#include "Defs.h"
#include "Http.h"
#include "Config.h"

#include <boost/asio.hpp>
#include <bitset>

#include <zlib.h>

class HttpHeaders;

class HttpRequest;

namespace Http {
  enum class ErrorCode : short;
    enum class Method : char;
  };

  class HttpReply : public std::enable_shared_from_this<HttpReply> {

public:
  enum Flag {
    Closing,
    GzipEncoding,
    ChunkedEncoding
  };

  HttpReply(HttpRequestPtr request);
  ~HttpReply();

  void process();

  const std::vector<boost::asio::const_buffer>& buffers() const;

  void setFlag(Flag flag, bool value);

  bool getFlag(Flag flag) const;

private:
  void handleGetRequest();
  void handleHeadRequest();

  void post(std::stringstream& stream);
  void buildErrorResponse(Http::ErrorCode error, std::stringstream& response, bool closeConnection = false);

  bool chunkResponse(std::vector<boost::asio::const_buffer>& buffers);
  bool encodeResponse(std::vector<boost::asio::const_buffer>& buffers);

  void buildHeaders();
  void initGzip();
  void setMimeType(const std::string& filename);
  bool requestFileContents(Http::Method method, const SiteConfig* site, std::stringstream& stream, size_t& filesize);

  boost::asio::const_buffer buf(const std::string& s);
  boost::asio::const_buffer buf(char* buf, size_t s);

  bool canChunkResponse() const;
  bool canEncodeResponse() const;

private:
  HttpRequestPtr m_request;
  HttpHeaders* m_replyHeaders;
  Http::ErrorCode m_status;

  std::bitset<3> m_flags;
  std::vector<boost::asio::const_buffer> m_buffers;
  std::vector<std::string> m_bufs;
  std::vector<char*> m_bufs2;
  size_t m_contentLength;

  z_stream m_gzip;
  bool m_gzipBusy;
};

#endif // HTTP_REPLY_H__

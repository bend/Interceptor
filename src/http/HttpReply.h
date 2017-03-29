#ifndef HTTP_REPLY_H__
#define HTTP_REPLY_H__

#include "Defs.h"
#include "Http.h"

#include <boost/asio.hpp>
#include <bitset>

class HttpHeaders;

class HttpReply : public std::enable_shared_from_this<HttpReply>{

public:
  enum Flag {
	Closing,
	Encoded,
	Chunked
  };

  HttpReply(HttpRequestPtr request);
  ~HttpReply();

  void process();

  const std::vector<boost::asio::const_buffer>& buffers() const;

  void setFlag(Flag flag, bool value);

  bool getFlag(Flag flag) const;

private:
  void handleGetRequest();
  void post(std::stringstream& stream);
  void buildErrorResponse(Http::ErrorCode error, std::stringstream& response, bool closeConnection = false);
  void chunkResponse(std::stringstream& stream);
  void encodeResponse(std::stringstream& stream);
  void buildHeaders();

private:
  HttpRequestPtr m_request;
  HttpHeaders* m_replyHeaders;
  Http::ErrorCode m_status;

  std::bitset<3> m_flags;
  std::vector<boost::asio::const_buffer> m_buffers;

};

#endif // HTTP_REPLY_H__

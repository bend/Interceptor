#ifndef HTTP_REPLY_H__
#define HTTP_REPLY_H__

#include "Defs.h"
#include "Http.h"

#include <boost/asio.hpp>

class HttpHeaders;

class HttpReply {

public:
  HttpReply(HttpRequestPtr request);
  ~HttpReply();

  void process();

  void buildHeader();

private:
  void handleGetRequest();
  void post(std::stringstream& stream);
  void buildErrorResponse(Http::ErrorCode error, std::stringstream& response, bool closeConnection = false);
  void chunkResponse(std::stringstream& stream);
  void encodeResponse(std::stringstream& stream);

private:
  HttpRequestPtr m_request;
  HttpHeaders* m_replyHeaders;
  Http::ErrorCode m_status;
  bool m_close;
  bool m_chunked;

};

#endif // HTTP_REPLY_H__

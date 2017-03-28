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

private:
  void handleGetRequest();
  void send(std::stringstream& stream);
  void sendErrorResponse(Http::ErrorCode error, std::stringstream& response, bool closeConnection = false);

private:
  HttpRequestPtr m_request;
  HttpHeaders* m_replyHeaders;
  std::string m_response;

};

#endif // HTTP_REPLY_H__

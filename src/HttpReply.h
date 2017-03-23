#ifndef HTTP_REPLY_H__
#define HTTP_REPLY_H__

#include "Defs.h"

#include <boost/asio.hpp>

class HttpReply {

public:
  HttpReply(HttpRequestPtr request);

  void process();

private:
  void handleGetRequest();

private:
  HttpRequestPtr m_request;
  std::string m_response;
  void handleHttpResponseSent(const boost::system::error_code& error, size_t bytesTransferred);

};

#endif // HTTP_REPLY_H__

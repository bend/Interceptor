#ifndef HTTP_REQUEST_H__
#define HTTP_REQUEST_H__

#include "Defs.h"

class HttpHeaders;

class HttpRequest {
public:
  enum Method {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATCH
  };

  HttpRequest(InterceptorSessionPtr session);
  ~HttpRequest();

  void appendData(const unsigned char* data, size_t length);

  Method method() const;
  Host host() const;
  std::string index() const;
  std::string httpVersion() const;
  InterceptorSessionPtr session() const;

  void process();
  bool headersReceived() const;
  bool completed() const;
  void setCompleted(bool completed);

private:
  void parse();
  void parseMethod(const std::string& method);
  void parseParameters(const std::string& parameters);

private:
  Method m_method;
  std::string m_index;
  std::string m_request;
  std::string m_httpVersion;
  InterceptorSessionPtr m_session;
  HttpHeaders* m_headers;
  bool m_completed;

  friend class HttpReply;

};

#endif //HTTP_REQUEST_H__

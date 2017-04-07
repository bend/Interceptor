#ifndef HTTP_REQUEST_H__
#define HTTP_REQUEST_H__

#include "Defs.h"
#include "Http.h"
#include "core/Config.h"

class HttpHeaders;

class HttpRequest {
public:
  HttpRequest(InterceptorSessionPtr session);
  ~HttpRequest();

  void appendData(const unsigned char* data, size_t length);

  Http::Method method() const;
  Host host() const;
  std::string index() const;
  std::string httpVersion() const;
  InterceptorSessionPtr session() const;
  std::string queryString() const;
  bool supportsCompression() const;
  bool supportsChunking() const;
  bool partialRequest() const;
  std::tuple<int64_t, int64_t> getRangeRequest() const;

  void process();
  bool headersReceived() const;
  bool completed() const;
  void setCompleted(bool completed);
  bool hasMatchingSite() const;
  const SiteConfig* matchingSite() const;

private:
  Http::Code parse();
  bool parseMethod(const std::string& method);
  bool parseParameters(const std::string& parameters);
  bool parseHttpVersion(const std::string& version);

private:
  Http::Method m_method;
  std::string m_index;
  std::string m_request;
  std::string m_httpVersion;
  InterceptorSessionPtr m_session;
  HttpHeaders* m_headers;
  bool m_completed;
  Host m_host;

  friend class HttpReply;

};

#endif //HTTP_REQUEST_H__

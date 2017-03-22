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

	void process();
	bool headersReceived() const;

  private:
	void parse();
	void parseMethod(const std::string& method);

  private:
	Method m_method;
	std::string m_index;
	std::string m_request;
	std::string m_httpVersion;
	InterceptorSessionPtr m_session;
	HttpHeaders *m_headers;
};

#endif //HTTP_REQUEST_H__

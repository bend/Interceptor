#ifndef HTTP_HEADERS_H__
#define HTTP_HEADERS_H__

#include <string>
#include <unordered_map>

class HttpHeaders {
public:
  HttpHeaders() = default;
  HttpHeaders(const std::string& headers);
  ~HttpHeaders() = default;

  void addHeader(const std::string& key, const std::string& value);
  const std::string* getHeader(const std::string& key) const;

private:
  void parse(const std::string& headers);

private:
  std::unordered_map<std::string, std::string> m_headers;


};

#endif // HTTP_HEADERS_H__

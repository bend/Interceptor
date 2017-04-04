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
  void addHeader(const std::string& key, int i);
  const std::string* getHeader(const std::string& key) const;

  void addGeneralHeaders();
  void serialize(std::stringstream& response) const;
  void fillFrom(const HttpHeaders* headers);

private:
  void parse(const std::string& headers);

private:
  std::unordered_map<std::string, std::string> m_headers;


};

#endif // HTTP_HEADERS_H__
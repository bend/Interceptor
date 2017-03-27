#include "HttpHeaders.h"

#include "Logger.h"
#include "Server.h"

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>


HttpHeaders::HttpHeaders(const std::string& headers)
{
  parse(headers);
}


void HttpHeaders::parse(const std::string& headers)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep1("\r\n");
  tokenizer tokens(headers, sep1);
  for (auto header : tokens) {
    size_t pos = header.find_first_of(":");
    if (pos == std::string::npos) {
      trace("error") << "HttpHeaders missing separator.. aborting for " << header;
      //TODO handle error 422
      return;
    }
    std::string part1 = header.substr(0, pos);
    std::string part2 = header.substr(pos + 1);
    m_headers[boost::trim_copy(part1)] = boost::trim_copy(part2);
    trace("debug") << "Parsing header " << part1 << ":" << part2;
  }
}

const std::string* HttpHeaders::getHeader(const std::string& key) const
{
  if (m_headers.count(key) > 0)
    return &m_headers.at(key);
  return nullptr;
}

void HttpHeaders::addHeader(const std::string& key, const std::string& value)
{
  m_headers[key] = value;
}

void HttpHeaders::addHeader(const std::string& key, int i)
{
  addHeader(key, std::to_string(i));
}

void HttpHeaders::addGeneralHeaders()
{
  addHeader("Server", Server::getCommonName());
}

void HttpHeaders::serialize(std::stringstream& response) const
{
  for (const auto& kv : m_headers) {
    response << kv.first << ": " << kv.second << "\r\n";
  }
  response << "\r\n";
}

#include "HttpHeaders.h"

#include "utils/Logger.h"
#include "utils/Server.h"

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

namespace Http {

  HttpHeaders::HttpHeaders(const std::string& headers)
    : m_request(headers)
  {}

  HttpHeaders::~HttpHeaders()
  {
    LOG_DEBUG("HttpHeaders::~HttpHeaders()");
  }


  Code HttpHeaders::parse()
  {
    const std::string headers = m_request;
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep1("\r\n");
    tokenizer tokens(headers, sep1);

    for (auto header : tokens) {
      size_t pos = header.find_first_of(":");

      if (pos == std::string::npos) {
        LOG_ERROR("HttpHeaders missing separator.. aborting for " << header);
        return Code::UnprocessableEntity;
      }

      std::string part1 = header.substr(0, pos);
      std::string part2 = header.substr(pos + 1);
      m_headers[boost::trim_copy(part1)] = boost::trim_copy(part2);
      LOG_DEBUG("Parsing header " << part1 << ":" << part2);
    }

    return Code::Ok;
  }

  const std::string* HttpHeaders::getHeader(const std::string& key) const
  {
    if (m_headers.count(key) > 0) {
      return &m_headers.at(key);
    }

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
    addHeader("Accept-Ranges", "bytes");
  }

  void HttpHeaders::serialize(std::stringstream& response) const
  {
    for (const auto& kv : m_headers) {
      response << kv.first << ": " << kv.second << "\r\n";
    }

    response << "\r\n";
  }

  void HttpHeaders::fillFrom(const HttpHeaders* headers)
  {
    if (headers->getHeader("X-Forwarded-For")) {
      addHeader("X-Forwarded-For", *headers->getHeader("X-Forwarded-For"));
    }

    if (headers->getHeader("Connection")) {
      addHeader("Connection", *headers->getHeader("Connection"));
    }
  }

}

#include "Headers.h"

#include "HttpException.h"

#include "utils/Logger.h"
#include "utils/ServerInfo.h"
#include "cache/generic_cache.h"
#include "utils/FileUtils.h"

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>


namespace Interceptor::Http {

  Headers::Headers(const std::string& headers)
    : m_request(headers)
  {}

  Headers::~Headers()
  {
    LOG_DEBUG("Headers::~Headers()");
  }


  void Headers::parse()
  {
    const std::string headers = m_request;
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep1("\r\n");
    tokenizer tokens(headers, sep1);

    for (auto header : tokens) {
      size_t pos = header.find_first_of(":");

      if (pos == std::string::npos) {
        LOG_ERROR("Headers missing separator.. aborting for " << header);
        throw HttpException(StatusCode::UnprocessableEntity);
      }

      std::string part1 = header.substr(0, pos);
      std::string part2 = header.substr(pos + 1);
      m_headers[boost::trim_copy(part1)] = boost::trim_copy(part2);
      LOG_DEBUG("Parsing header " << part1 << ":" << part2);
    }
  }

  const std::string* Headers::getHeader(const std::string& key) const
  {
    if (m_headers.count(key) > 0) {
      return &m_headers.at(key);
    }

    return nullptr;
  }

  void Headers::addHeader(const std::string& key, const std::string& value)
  {
    m_headers[key] = value;
  }

  void Headers::addHeader(const std::string& key, int i)
  {
    addHeader(key, std::to_string(i));
  }

  void Headers::addGeneralHeaders()
  {
    addHeader("Date", ServerInfo::currentDate());
    addHeader("Server", ServerInfo::commonName());
    addHeader("Accept-Ranges", "bytes");
  }

  void Headers::serialize(std::stringstream& response) const
  {
    for (const auto& kv : m_headers) {
      response << kv.first << ": " << kv.second << "\r\n";
    }

    response << "\r\n";
  }

  void Headers::fillFrom(const Headers* headers)
  {
    if (headers->getHeader("X-Forwarded-For")) {
      addHeader("X-Forwarded-For", *headers->getHeader("X-Forwarded-For"));
    }

    if (headers->getHeader("Connection")) {
      addHeader("Connection", *headers->getHeader("Connection"));
    }
  }

  void Headers::setHeadersFor(const std::string& filename,
                              AbstractCacheHandler* cache)
  {
    addHeader("Content-Type", FileUtils::mimeType(filename));

    std::string eTag = cache->eTag(filename);

    if (eTag.length() > 0) {
      addHeader("ETag", eTag);
    }

    std::string lm = cache->lastModified(filename);

    if (lm.length() > 0) {
      addHeader("Last-Modified", lm);
    }

  }

}

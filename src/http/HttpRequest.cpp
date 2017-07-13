#include "HttpRequest.h"

#include "core/InterceptorSession.h"
#include "HttpHeaders.h"
#include "utils/Logger.h"
#include "utils/FileBuffer.h"
#include "common/Params.h"
#include "cache/generic_cache.h"

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <regex>

namespace Http {

  HttpRequest::HttpRequest(InterceptorSessionWeakPtr session)
    : m_session(session),
      m_method(Method::ERR),
      m_headers(nullptr),
      m_completed(false),
      m_parsed(false),
      m_dumpingToFile(false),
      m_host(""),
	  m_buffer(nullptr)
  {
  }

  HttpRequest::~HttpRequest()
  {
    LOG_DEBUG("HttpRequest::~HttpRequest()");
    delete m_headers;
  }

  Code HttpRequest::appendData(const unsigned char* data, size_t length)
  { 
	try {
    LOG_DEBUG("HttpRequest::appendData()");
    m_request.append(std::string(data, data + length));

    uint64_t mrs =
      m_session.lock()->params()->config()->m_globalConfig->maxRequestSize();
    uint64_t mirs =
      m_session.lock()->params()->config()->m_globalConfig->maxInMemRequestSize();

    if (mrs > 0  &&( m_request.length() > mrs
    || (m_dumpingToFile && m_buffer->size() > mrs))) {
      return Code::RequestEntityTooLarge;
    }

    if ((/*mirs > 0*&& */m_request.length() > mirs) || m_dumpingToFile) {
	  if(!m_dumpingToFile) {
		dumpToFile(reinterpret_cast<const unsigned char*>(m_request.c_str()), m_request.length());
		m_request.clear();
	  } else 
		dumpToFile(data, length);
    }

    LOG_DEBUG(m_request);

	}catch(std::exception e)
	 {
		LOG_ERROR("Exception");
	  return Code::InternalServerError;
	 }
	 return Code::Ok;
  }

  bool HttpRequest::headersReceived() const
  {
    return m_dumpingToFile ? m_buffer->headersReceived() : m_request.find("\r\n\r\n") != std::string::npos; 
  }

  Host HttpRequest::host() const
  {
    return m_host;
  }

  Method HttpRequest::method() const
  {
    return m_method;
  }

  std::string HttpRequest::index() const
  {
    return m_index;
  }

  std::string HttpRequest::httpVersion() const
  {
    return m_httpVersion;
  }

  InterceptorSessionPtr HttpRequest::session() const
  {
    return m_session.lock();
  }

  AbstractCacheHandler* HttpRequest::cacheHandler() const
  {
    return m_session.lock()->params()->cache();
  }

  bool HttpRequest::completed() const
  {
    return m_completed;
  }

  std::string HttpRequest::queryString() const
  {
    return m_request;
  }

  void HttpRequest::setCompleted(bool completed)
  {
    m_completed = completed;
    m_endTs = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>
                    ( m_endTs - m_startTs ).count();
    LOG_INFO(queryString() << "- processing time : " << duration << " ms");
  }

  bool HttpRequest::supportsCompression() const
  {
    const std::string* ae = m_headers->getHeader("Accept-Encoding");
    return ae && ae->find("gzip") != std::string::npos;
  }

  bool HttpRequest::supportsChunking() const
  {
    return m_httpVersion == "1.1" || supportsCompression();
  }

  bool HttpRequest::partialRequest() const
  {
    const std::string* pr = m_headers->getHeader("Range");
    return pr && pr->find("bytes=") != std::string::npos;
  }

  Code HttpRequest::getRangeRequest(std::tuple<int64_t, int64_t>& tuple) const
  {
    const std::string* pr = m_headers->getHeader("Range");

    if (!pr)
      return {};

    size_t pos = pr->find("bytes=");

    if (pos == std::string::npos) {
      return Code::BadRequest;
    }

    std::string range = pr->substr(pos + 6);

    if (range.find("-") == std::string::npos) {
      return Code::BadRequest;
    }

    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

    boost::char_separator<char> sep("-", "", boost::keep_empty_tokens);

    std::vector<int64_t> vals;

    for (auto& token : tokenizer(range, sep)) {
      if (token.length() > 0) {
        try {
          vals.push_back(std::stoi(boost::trim_copy(token)));
        } catch (std::out_of_range) {
          return Code::RequestRangeNotSatisfiable;
        } catch (std::invalid_argument) {
          return Code::BadRequest;
        }
      } else {
        vals.push_back(-1);
      }

    }

    std::get<0>(tuple) = vals[0];
    std::get<1>(tuple) = vals[1];
    return Code::Ok;
  }

  std::string HttpRequest::headersData()
  {
	if(m_dumpingToFile)  
	  return m_buffer->headersData();

	size_t pos = m_request.find("\r\n\r\n");

	return m_request.substr(0, pos);
  }

  Code HttpRequest::parse()
  {
    LOG_DEBUG("HttpRequest::parse()");
    m_startTs = std::chrono::high_resolution_clock::now();
	std::string headers = headersData();
    size_t pos = headers.find_first_of("\r\n");

    if (pos == std::string::npos) {
      LOG_ERROR("HttpRequest missing separator.. aborting");
      return Code::BadRequest;
    }

    std::string get = headers.substr(0, pos);
    headers= headers.substr(pos);
    std::vector<std::string> getParts;
    boost::split(getParts, get , boost::is_any_of(" "));

    if (getParts.size() != 3) {
      LOG_ERROR("Missing Method part");
      return Code::BadRequest;
    }

    if (!parseMethod(getParts[0])) {
      return Code::BadRequest;
    }

    switch (m_method) {
      case Method::GET: {
          size_t st = getParts[1].find("?");

          if (st != std::string::npos) {
            parseParameters(getParts[1].substr(st));
            getParts[1] = getParts[1].substr(0, st);
          }
        }
        break;

      case Method::POST:
      case Method::PUT:
      case Method::DELETE:
      case Method::TRACE:
      case Method::OPTIONS:
      case Method::CONNECT:
      case Method::PATCH:
        return Http::Code::NotImplemented;
        break;

      default:
        break;
    }

    m_index = getParts[1];

    if (!parseHttpVersion(getParts[2])) {
      return Code::HttpVersionNotSupported;
    }

    m_headers = new HttpHeaders(headers);

    Code code = m_headers->parse();

    if (code != Code::Ok) {
      return code;
    }

    m_request = get;

    // parse host
    const std::string* host = m_headers->getHeader("Host");

    if (!host ) {
      LOG_ERROR("Missing Host" );
      return Code::BadRequest;
    }

    size_t spos = host->find(":");
    m_host = host->substr(0, spos);
    return Code::Ok;
  }

  bool HttpRequest::parseHttpVersion(const std::string& version)
  {
    const std::string http = "HTTP/";
    size_t idx = version.find(http);

    if (idx != 0) {
      return false;
    }

    m_httpVersion = version.substr(idx + http.length());

    if (m_httpVersion == "1.0" || m_httpVersion == "1.1") {
      return true;
    }

    return false;
  }

  bool HttpRequest::parseMethod(const std::string& method)
  {
    if (method == "GET") {
      m_method = Method::GET;
      return true;
    } else if (method == "HEAD") {
      m_method = Method::HEAD;
      return true;
    } else if (method == "POST") {
      m_method = Method::POST;
      return true;
    }

    return false;
  }

  bool HttpRequest::parseParameters(const std::string& params)
  {
    LOG_DEBUG("Parsing parameters " << params);
    return true;
  }

  bool HttpRequest::hasMatchingSite() const
  {
    return matchingSite() != nullptr;
  }

  const Config::ServerConfig::Site* HttpRequest::matchingSite() const
  {
    std::string host = m_host;

    if (Config::isLocalDomain(host)) {
      host = Config::replaceLocalDomain(host);
    }

    for (const auto& site : session()->params()->config()->m_sites) {
      if (site->m_host == host) {
        return site;
      }
    }

    return nullptr;
  }

  bool HttpRequest::parsed()
  {
    return m_parsed;
  }

  bool HttpRequest::dumpToFile(const unsigned char* data, size_t length)
  {
	if(!m_buffer) {
	  m_buffer = std::make_unique<FileBuffer>();
	  m_dumpingToFile = true;
	}
  
	m_buffer->append(data, length);

	return true;
  }

}

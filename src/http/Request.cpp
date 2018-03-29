#include "Request.h"

#include "Headers.h"
#include "HttpException.h"
#include "core/SessionConnection.h"
#include "utils/Logger.h"
#include "common/FileBuffer.h"
#include "common/Params.h"
#include "common/Packet.h"
#include "cache/generic_cache.h"
#include "vars.h"

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <regex>
#include <utility>

namespace Interceptor::Http {

  Request::Request(SessionConnectionPtr connection)
    : m_connection(connection),
      m_method(Method::ERR),
      m_headers(nullptr),
      m_state(0x0),
      m_host("")
  {
  }

  Request::~Request()
  {
    LOG_DEBUG("Request::~Request()");
  }

  bool Request::requestSizeIsAccepted() const
  {
    uint64_t mrs =
      params()->config()->m_globalConfig->maxRequestSize();

    return mrs > 0 && ( m_request.length() <= mrs || (m_state.test(Dumping)
                        && m_buffer->size() <= mrs));
  }

  bool Request::requestSizeFitsInMemory() const
  {
    uint64_t mirs =
      params()->config()->m_globalConfig->maxInMemRequestSize();
    return mirs > 0 && (m_request.length() <= mirs && !m_state.test(Dumping));
  }

  void Request::appendData(const char* data, size_t length)
  {
    try {
      LOG_DEBUG("Request::appendData()");
      m_request.append(std::string(data, data + length));

      if (!requestSizeIsAccepted()) {
        throw HttpException(StatusCode::RequestEntityTooLarge, true);
      }

      if (!requestSizeFitsInMemory()) {
        if (!m_state.test(Dumping)) {
          dumpToFile(m_request.c_str(),
                     m_request.length());
        } else {
          dumpToFile(data, length);
        }

        // set the data in m_request for big request to be able to forward data
        m_request.clear();
        pushRequest(data, length);
      }

      LOG_NETWORK("Request: ", m_request);

    } catch (std::exception& e) {
      LOG_ERROR("Exception" << e.what());
      throw HttpException(StatusCode::InternalServerError, true);
    }
  }

  bool Request::headersReceived() const
  {
    return m_state.test(Dumping) ? m_buffer->headersReceived() :
           m_request.find("\r\n\r\n") != std::string::npos;
  }

  Host Request::host() const
  {
    return m_host;
  }

  Method Request::method() const
  {
    return m_method;
  }

  std::string Request::index() const
  {
    return m_index;
  }

  std::string Request::httpVersion() const
  {
    return m_httpVersion;
  }

  SessionConnectionPtr Request::connection() const
  {
    return m_connection;
  }

  ParamsPtr Request::params() const
  {
    return m_connection->params();
  }

  Cache::AbstractCacheHandler* Request::cacheHandler() const
  {
    return m_connection->params()->cache();
  }

  bool Request::completed() const
  {
    return m_state.test(Completed);
  }

  std::string Request::queryString() const
  {
    size_t pos = m_request.find("\r\n");

    if ( pos == std::string::npos) {
      return "";
    }

    return m_request.substr(0, pos);
  }

  void Request::setCompleted(bool completed)
  {
    m_state.set(Completed);
    m_endTs = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>
                    ( m_endTs - m_startTs ).count();
    LOG_INFO(queryString() << "- processing time : " << duration << " ms");
  }

  bool Request::supportsCompression() const
  {
    const std::string* ae = m_headers->getHeader("Accept-Encoding");
    return ae && ae->find("gzip") != std::string::npos;
  }

  bool Request::supportsChunking() const
  {
    return m_httpVersion == "1.1" || supportsCompression();
  }

  bool Request::partialRequest() const
  {
    const std::string* pr = m_headers->getHeader("Range");
    return pr && pr->find("bytes=") != std::string::npos;
  }

  bool Request::hasIfModifiedSince() const
  {
    return m_headers->getHeader("If-Modified-Since");
  }

  std::time_t Request::ifModifiedSince() const
  {
    auto timeStr = getHeader("If-Modified-Since");

    std::array<std::string, 3> formats = {
      "%a %b %d %H:%M:%S %Y",		// Fri Dec 31 23:59:59 1999
      "%a, %d %b %Y %H:%M:%S GMT",	// Fri, 31 Dec 1999 23:59:59 GMT
      "%aday, %d-%b-%y %H:%M:%S GMT",	// Friday, 31-Dec-99 23:59:59 GMT
    };

    for (auto format : formats) {
      std::istringstream str_stream(*timeStr);
      std::tm tm{};
      str_stream >> std::get_time(&tm, format.c_str());

      if (str_stream.fail()) {
        continue;
      }

      std::time_t time = std::mktime(&tm);
      LOG_DEBUG("TIME : " << time);
      return time;
    }

    return -1;
  }

  bool Request::closeConnection() const
  {
    if (!m_headers) {
      return false;
    }

    const std::string* connection =  m_headers->getHeader("Connection");
    return connection && *connection != "keep-alive";
  }

  std::tuple<int64_t, int64_t> Request::getRangeRequest() const
  {
    std::tuple<int64_t, int64_t> tuple;
    const std::string* pr = m_headers->getHeader("Range");

    if (!pr)
      return {};

    size_t pos = pr->find("bytes=");

    if (pos == std::string::npos) {
      throw HttpException(StatusCode::BadRequest, true);
    }

    std::string range = pr->substr(pos + 6);

    if (range.find("-") == std::string::npos) {
      throw HttpException(StatusCode::BadRequest, true);
    }

    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

    boost::char_separator<char> sep("-", "", boost::keep_empty_tokens);

    std::vector<int64_t> vals;

    for (auto& token : tokenizer(range, sep)) {
      if (token.length() > 0) {
        try {
          vals.push_back(std::stoi(boost::trim_copy(token)));
        } catch (std::out_of_range) {
          throw HttpException(StatusCode::RequestRangeNotSatisfiable, true);
        } catch (std::invalid_argument) {
          throw HttpException(StatusCode::BadRequest, true);
        }
      } else {
        vals.push_back(-1);
      }

    }

    std::get<0>(tuple) = vals[0];
    std::get<1>(tuple) = vals[1];
    return tuple;
  }

  std::string Request::headersData()
  {
    if (m_state.test(Dumping)) {
      return m_buffer->headersData();
    }

    size_t pos = m_request.find("\r\n\r\n");

    return m_request.substr(0, pos);
  }

  void Request::parse()
  {
    LOG_DEBUG("Request::parse()");
    LOG_NETWORK("Request:", m_request);
    m_startTs = std::chrono::high_resolution_clock::now();
    std::string headers = headersData();
    size_t pos = headers.find_first_of("\r\n");

    if (pos == std::string::npos) {
      LOG_ERROR("Request missing separator.. aborting");
      throw HttpException(StatusCode::BadRequest);
    }

    std::string get = headers.substr(0, pos);
    headers = headers.substr(pos);
    std::vector<std::string> getParts;
    boost::split(getParts, get, boost::is_any_of(" "));

    if (getParts.size() != 3) {
      LOG_ERROR("Missing Method part");
      throw HttpException(StatusCode::BadRequest);
    }

    parseMethod(getParts[0]);

    parseHttpVersion(getParts[2]);

    switch (m_method) {
      case Method::GET: {
          size_t st = getParts[1].find("?");

          if (st != std::string::npos) {
            parseParameters(getParts[1].substr(st));
            getParts[1] = getParts[1].substr(0, st);
          }
        }
        break;

      case Method::HEAD:
      case Method::POST:
        break;

      case Method::PUT:
      case Method::DELETE:
      case Method::TRACE:
      case Method::OPTIONS:
      case Method::CONNECT:
      case Method::PATCH:
        throw HttpException(StatusCode::NotImplemented);
        break;

      default:
        break;
    }

    m_index = getParts[1];

    m_headers = std::make_unique<Headers>(headers);

    m_headers->parse();

    // parse host
    const std::string* host = getHeader("Host");

    if (!host) {
      LOG_ERROR("Missing Host" );
      throw HttpException(StatusCode::BadRequest);
    }

    size_t spos = host->find(":");
    m_host = host->substr(0, spos);

    // TODO parse body, check that everything is received
    m_state.set(Received); // TODO FIXME check if actually received
  }

  void Request::parseHttpVersion(const std::string& version)
  {
    const std::string http = "HTTP/";
    size_t idx = version.find(http);

    if (idx != 0) {
      throw HttpException(StatusCode::BadRequest);
    }

    m_httpVersion = version.substr(idx + http.length());

    if (m_httpVersion != "1.1") {
      throw HttpException(StatusCode::HttpVersionNotSupported);
    }

  }

  void Request::parseMethod(const std::string& method)
  {
    if (method == "GET") {
      m_method = Method::GET;
    } else if (method == "HEAD") {
      m_method = Method::HEAD;
    } else if (method == "POST") {
      m_method = Method::POST;
    } else if (method == "DELETE") {
      m_method = Method::DELETE;
    } else {
      throw HttpException(StatusCode::BadRequest);
    }
  }

  void Request::parseParameters(const std::string& params)
  {
    LOG_DEBUG("Parsing parameters " << params);
  }

  bool Request::hasMatchingSite() const
  {
    return matchingSite() != nullptr;
  }

  const Config::ServerConfig::Site* Request::matchingSite() const
  {
    std::string host = m_host;

    if (Config::isLocalDomain(host)) {
      host = Config::replaceLocalDomain(host);
    }

    for (const auto& site : params()->config()->m_sites) {
      if (site->m_host == host) {
        return site;
      }
    }

    return nullptr;
  }

  bool Request::parsed()
  {
    return m_state.test(Parsed);
  }

  bool Request::dumpToFile(const char* data, size_t length)
  {
    if (!m_buffer) {
      m_buffer = std::make_unique<FileBuffer>();
      m_state.set(Dumping);
    }

    m_buffer->append(data, length);

    return true;
  }

  Packet* Request::request()
  {
    if (m_state.test(Dumping)) {
      return popRequest();
    }

    char* cp = new char[m_request.length()]();
    std::strncpy(cp, m_request.c_str(), m_request.length());
    return new Packet(cp, m_request.length());
  }

  void Request::pushRequest(const char* data, size_t length)
  {
    char* cp = new char[length]();
    std::strncpy(cp, data, length);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(new Packet(cp, length));
    m_sig(); // notify listeners that we have more data
  }

  Packet* Request::popRequest()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto p = m_queue.front();
    m_queue.pop();
    return p;
  }

  bool Request::hasData()
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size() > 0;
  }

  bool Request::received() const
  {
    return m_state.test(Received);
  }

  boost::signals2::signal<void()>& Request::hasMoreData()
  {
    return m_sig;
  }

  const std::string* Request::getHeader(const std::string& header) const
  {
    if (!m_headers) {
      return nullptr;
    }

    auto value = m_headers->getHeader(header);

    if (value) {
      return value;
    } else  {
      std::string hdr = boost::algorithm::to_lower_copy(header);
      return m_headers->getHeader(hdr);
    }
  }

}

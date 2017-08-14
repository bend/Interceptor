#include "Request.h"

#include "core/SessionConnection.h"
#include "Headers.h"
#include "utils/Logger.h"
#include "common/FileBuffer.h"
#include "common/Params.h"
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

  Code Request::appendData(const char* data, size_t length)
  {
    try {
      LOG_DEBUG("Request::appendData()");
      m_request.append(std::string(data, data + length));

      uint64_t mrs =
        params()->config()->m_globalConfig->maxRequestSize();
      uint64_t mirs =
        params()->config()->m_globalConfig->maxInMemRequestSize();

      if (mrs > 0  && ( m_request.length() > mrs
                        || (m_state.test(Dumping) && m_buffer->size() > mrs))) {
        return Code::RequestEntityTooLarge;
      }

      if ((/*mirs > 0*&& */m_request.length() > mirs) || m_state.test(Dumping)) {
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

      LOG_DEBUG(m_request);

    } catch (std::exception e) {
      LOG_ERROR("Exception" << e.what());
      return Code::InternalServerError;
    }

    return Code::Ok;
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

  AbstractCacheHandler* Request::cacheHandler() const
  {
    return m_connection->params()->cache();
  }

  bool Request::completed() const
  {
    return m_state.test(Completed);
  }

  std::string Request::queryString() const
  {
    return m_request;
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

  Code Request::getRangeRequest(std::tuple<int64_t, int64_t>& tuple) const
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

  std::string Request::headersData()
  {
    if (m_state.test(Dumping)) {
      return m_buffer->headersData();
    }

    size_t pos = m_request.find("\r\n\r\n");

    return m_request.substr(0, pos);
  }

  Code Request::parse()
  {
    LOG_DEBUG("Request::parse()");
    LOG_NETWORK("Request:", m_request);
    m_startTs = std::chrono::high_resolution_clock::now();
    std::string headers = headersData();
    size_t pos = headers.find_first_of("\r\n");

    if (pos == std::string::npos) {
      LOG_ERROR("Request missing separator.. aborting");
      return Code::BadRequest;
    }

    std::string get = headers.substr(0, pos);
    headers = headers.substr(pos);
    std::vector<std::string> getParts;
    boost::split(getParts, get , boost::is_any_of(" "));

    if (getParts.size() != 3) {
      LOG_ERROR("Missing Method part");
      return Code::BadRequest;
    }

    if (!parseMethod(getParts[0])) {
      return Code::BadRequest;
    }

    if (!parseHttpVersion(getParts[2])) {
      return Code::HttpVersionNotSupported;
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
        return Code::NotImplemented;
        break;

      default:
        break;
    }

    m_index = getParts[1];

    m_headers = std::make_unique<Headers>(headers);

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

    // TODO parse body, check that everything is received
    m_state.set(Received); // TODO FIXME check if actually received
    return Code::Ok;
  }

  bool Request::parseHttpVersion(const std::string& version)
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

  bool Request::parseMethod(const std::string& method)
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
    } else if (method == "DELETE") {
      m_method = Method::DELETE;
      return true;
    }

    return false;
  }

  bool Request::parseParameters(const std::string& params)
  {
    LOG_DEBUG("Parsing parameters " << params);
    return true;
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

  Packet Request::request()
  {
    if (m_state.test(Dumping)) {
      return popRequest();
    }

    return std::make_pair(m_request.data(), m_request.length());
  }

  void Request::pushRequest(const char* data, size_t length)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::make_pair(data, length));
    m_sig(); // notify listeners that we have more data
  }

  Packet Request::popRequest()
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

}
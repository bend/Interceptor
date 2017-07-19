#ifndef HTTP_REQUEST_H__
#define HTTP_REQUEST_H__

#include "common/Defs.h"
#include "Http.h"
#include "core/Config.h"

#include <chrono>
#include <queue>

class AbstractCacheHandler;
class FileBuffer;

namespace Http {
  class HttpHeaders;

  class HttpRequest {
  public:
    HttpRequest(InterceptorSessionWeakPtr session);
    ~HttpRequest();

    Code appendData(const char* data, size_t length);

    Method method() const;
    Host host() const;
    std::string index() const;
    std::string httpVersion() const;
    InterceptorSessionPtr session() const;
    AbstractCacheHandler* cacheHandler() const;
    std::string queryString() const;
    bool supportsCompression() const;
    bool supportsChunking() const;
    bool partialRequest() const;
    Code getRangeRequest(std::tuple<int64_t, int64_t>& range) const;

    void process();
    bool headersReceived() const;
    bool completed() const;
    void setCompleted(bool completed);
    bool hasMatchingSite() const;
    const SiteConfig* matchingSite() const;
	const char* request() const;
	size_t length() const;

  private:
    Code parse();
    bool parsed();
    bool parseMethod(const std::string& method);
    bool parseParameters(const std::string& parameters);
    bool parseHttpVersion(const std::string& version);
    bool dumpToFile(const char* data, size_t length);
    std::string headersData();

  private:
    InterceptorSessionWeakPtr m_session;
    Method m_method;
    std::string m_index;
    std::string m_request;
    std::string m_httpVersion;
    HttpHeaders* m_headers;
    bool m_completed;
    bool m_parsed;
    bool m_dumpingToFile;
    Host m_host;
    std::unique_ptr<FileBuffer> m_buffer;

    std::chrono::high_resolution_clock::time_point m_startTs;
    std::chrono::high_resolution_clock::time_point m_endTs;
	std::queue<const char*> m_queue;

    friend class HttpReply;

  };

}

#endif //HTTP_REQUEST_H__

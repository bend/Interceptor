#ifndef HTTP_REQUEST_H__
#define HTTP_REQUEST_H__

#include "common/Defs.h"
#include "Http.h"
#include "core/Config.h"

#include <chrono>
#include <queue>
#include <mutex>
#include <bitset>

#include <boost/signals2.hpp>

namespace Interceptor {
  class AbstractCacheHandler;
  class FileBuffer;
  struct Packet;

  namespace Http {

    class Request {
    public:
      Request(SessionConnectionPtr session);
      ~Request();

      void appendData(const char* data, size_t length);
      void process();

      Method method() const;
      Host host() const;
      std::string index() const;
      std::string httpVersion() const;
      SessionConnectionPtr connection() const;
      ParamsPtr params() const;
      AbstractCacheHandler* cacheHandler() const;
      std::string queryString() const;
      bool supportsCompression() const;
      bool supportsChunking() const;
      bool partialRequest() const;
      std::time_t ifModifiedSince() const;
      std::tuple<int64_t, int64_t> getRangeRequest() const;

      bool headersReceived() const;
      bool received() const;
      bool completed() const;
      bool hasIfModifiedSince() const;
      bool closeConnection() const;
      void setCompleted(bool completed);
      bool hasMatchingSite() const;
      const std::string* getHeader(const std::string& header) const;

      const SiteConfig* matchingSite() const;
      Packet* request();
      Packet* popRequest();
      boost::signals2::signal<void()>& hasMoreData();
      bool hasData();

    private:
      void parse();
      bool parsed();
      void parseMethod(const std::string& method);
      void parseParameters(const std::string& parameters);
      void parseHttpVersion(const std::string& version);
      bool dumpToFile(const char* data, size_t length);
      bool requestSizeIsAccepted() const;
      bool requestSizeFitsInMemory() const;
      std::string headersData();
      void pushRequest(const char* data, size_t length);


    private:
      enum State {
        Completed,
        Parsed,
        Dumping,
        Received
      };

    private:
      SessionConnectionPtr m_connection;
      Method m_method;
      std::string m_index;
      std::string m_request;
      std::string m_httpVersion;
      HttpHeaderUPtr m_headers;
      std::bitset<4> m_state;
      Host m_host;
      std::unique_ptr<FileBuffer> m_buffer;
      std::chrono::high_resolution_clock::time_point m_startTs;
      std::chrono::high_resolution_clock::time_point m_endTs;
      std::queue<Packet*> m_queue;
      std::mutex m_mutex;
      boost::signals2::signal<void()> m_sig;

      friend class Reply;
    };

  }

}

#endif //HTTP_REQUEST_H__

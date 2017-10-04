#ifndef HTTP_HEADERS_H__
#define HTTP_HEADERS_H__

#include "Http.h"

#include <string>
#include <unordered_map>

namespace Interceptor {

  class AbstractCacheHandler;

  namespace Http {

    class Headers {
    public:
      Headers() = default;
      Headers(const std::string& headers);
      ~Headers();

      void addHeader(const std::string& key, const std::string& value);
      void addHeader(const std::string& key, int i);
      const std::string* getHeader(const std::string& key) const;

      void addGeneralHeaders();
      void serialize(std::stringstream& response) const;
      void fillFrom(const Headers* headers);
      void parse();

      void setHeadersFor(const std::string& filename, AbstractCacheHandler* cache);

    private:
      std::unordered_map<std::string, std::string> m_headers;
      std::string m_request;


    };

  }

}

#endif // HTTP_HEADERS_H__

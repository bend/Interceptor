#ifndef CONFIG_H__
#define CONFIG_H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include "json/json.hpp"

using json = nlohmann::json;

typedef std::map<std::string, std::string> ErrorPageMap;
typedef std::map<std::string, int16_t> LocationsMap;

class ConfigException : public std::exception {
public:
  ConfigException(std::string s):
    m_what(s) {}

  virtual ~ConfigException() noexcept {}

  virtual const char* what() noexcept
  {
    return m_what.c_str();
  }

private:
  std::string m_what;
};

class Config {

public:

  class ServerConfig {
  public:

    ServerConfig(const ErrorPageMap& map)
      : m_errorPages(map) {}

    struct Site {
      std::string m_host;
      std::string m_docroot;
      std::vector<std::string> m_tryFiles;
      std::set<std::string> m_gzip;
      ErrorPageMap m_errorPages;
	  LocationsMap m_locations;
    };

    std::string m_listenHost;
    int m_listenPort;

    std::vector<Site*> m_sites;
    const ErrorPageMap& m_errorPages;
    uint32_t m_clientTimeout;
    uint32_t m_serverTimeout;

  };

public:
  Config(const std::string& path);
  const std::vector<ServerConfig*> serversConfig() const;
  uint16_t threads() const;
  uint64_t maxCacheSize() const;

  static bool isLocalDomain(const std::string& domain);
  static std::string localDomain();
  static std::string replaceLocalDomain(const std::string& domain);

private:
  void parse();
  void parseErrorPages(json& j, ErrorPageMap& map, const std::string& appendPath);
  std::string parseDocRoot(const std::string& docroot) const;

private:
  const std::string m_path;
  std::string m_cwd;
  std::vector<ServerConfig*> m_serversConfig;

  /* Global section */
  ErrorPageMap m_errorPages;
  uint32_t m_clientTimeout;
  uint32_t m_serverTimeout;
  uint16_t m_threadNr;
  uint64_t m_maxCacheSize;

};

typedef Config::ServerConfig::Site SiteConfig;

#endif //CONFIG_H__

#ifndef CONFIG_H__
#define CONFIG_H__

#include <string>
#include <vector>
#include <map>
#include <set>
#include "json/json.hpp"
#include "backend/defs.h"
#include "modules/defs.h"
#include "authentication/defs.h"
#include "common/Redirection.h"

using json = nlohmann::json;

namespace Interceptor {

  typedef std::map<std::string, std::string> ErrorPageMap;
  typedef std::map<std::string, int16_t> LocationsMap;
  typedef std::map<std::string, bool> ListingMap;
  typedef std::map<std::string, BackendPtr> BackendsMap;
  typedef std::map<std::string, ConnectorPtr> ConnectorsMap;
  typedef std::map<std::string, ModulePtr> ModulesMap;
  typedef std::map<std::string, AuthenticationPtr> AuthenticationsMap;

  class ConfigException : public std::exception {
  public:
    ConfigException(std::string s):
      m_what(s) {}

    virtual ~ConfigException() noexcept {}

    virtual const char* what() const noexcept
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

      ~ServerConfig();

      struct Site {
        std::string m_host;
        std::string m_docroot;
        std::vector<std::string> m_tryFiles;
        std::set<std::string> m_gzip;
        ErrorPageMap m_errorPages;
        LocationsMap m_locations;
        ListingMap m_listing;
        std::string m_backend;
        std::map<std::string, std::string> m_connectors;
        std::map<std::string, std::string> m_modules;
        std::map<std::string, std::string> m_authenticators;
        std::map<std::string, std::string> m_realms;
        std::map<std::string, time_t> m_cacheTime;
        std::vector<Redirection> m_redirections;

        const time_t getCacheTime(const std::string& path) const;

        const std::string connectorName(const std::string& path) const;

        const std::string moduleName(const std::string& path) const;

        const std::string authenticatorName(const std::string& path) const;

        const Redirection* redirection(const std::string& path) const;

        const std::string realm(const std::string& path) const;

        const bool listingAllowed(const std::string& path) const;
      };

      std::string m_listenHost;
      int m_listenPort;
      bool m_useSSL;
      std::string m_pemfile;
      std::string m_dhfile;

      std::vector<Site*> m_sites;
      const ErrorPageMap& m_errorPages;
      uint32_t m_clientTimeout;
      uint32_t m_serverTimeout;
      Config* m_globalConfig;
    };

  public:
    Config(const std::string& path);
    ~Config();
    const std::vector<ServerConfig*> serversConfig() const;
    uint16_t threads() const;
    uint64_t maxCacheSize() const;
    uint64_t maxRequestSize() const;
    uint64_t maxInMemRequestSize() const;
    const BackendsMap& backends() const;
    const ConnectorsMap& connectors() const;
    const ModulesMap& modules() const;
    const AuthenticationsMap& authentications() const;

    static bool isLocalDomain(const std::string& domain);
    static std::string localDomain();
    static std::string replaceLocalDomain(const std::string& domain);
    static constexpr uint32_t  mbToBytesFactor();

  private:
    void parse();
    void parseErrorPages(json& j, ErrorPageMap& map, const std::string& appendPath);
    std::string parseDocRoot(const std::string& docroot) const;
    void parseBackends(json& j);
    void parseConnectors(json& j);
    void parseModules(json& j);
    void parseAuthentications(json& j);
    void parseLocations(json& j, ServerConfig::Site* s);
    void parseRedirections(json& j, ServerConfig::Site* s);

  private:
    static time_t parseTimeUnit(const std::string& time);
    static Redirection::Type toType(const std::string& type);

  private:
    const std::string m_path;
    std::string m_cwd;
    std::vector<ServerConfig*> m_serversConfig;

    /* Global section */
    ErrorPageMap m_errorPages;
    BackendsMap m_backends;
    ConnectorsMap m_connectors;
    ModulesMap m_modules;
    AuthenticationsMap m_authenticators;
    uint32_t m_clientTimeout;
    uint32_t m_serverTimeout;
    uint16_t m_threadNr;
    uint64_t m_maxCacheSize;
    uint64_t m_maxRequestSize;
    uint64_t m_maxInMemRequest;

  };

  typedef Config::ServerConfig::Site SiteConfig;

}

#endif //CONFIG_H__

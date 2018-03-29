#include "Config.h"

#include "vars.h"
#include "utils/Logger.h"
#include "utils/StringUtils.h"

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <thread>

namespace Interceptor {

  Config::Config(const std::string& path)
    : m_path(path),
      m_cwd(boost::filesystem::current_path().string()),
      m_clientTimeout(0),
      m_serverTimeout(0),
      m_maxCacheSize(0),
      m_maxRequestSize(0),
      m_maxInMemRequest(0)
  {
    parse();
  }

  Config::~Config()
  {
    for (auto& sc : m_serversConfig) {
      delete sc;
    }

    m_serversConfig.clear();
  }

  Config::ServerConfig::~ServerConfig()
  {
    for (auto& s : m_sites) {
      delete s;
    }

    m_sites.clear();
  }

  void Config::parse()
  {

    LOG_INFO("Config::parse() - Reading configuration file " << m_path);

    std::ifstream config(m_path);

    if (!config.is_open()) {
      throw ConfigException("Cannot open config file " + m_path);
    }

    try {
      // global section
      json j = json::parse(config);

      auto global = j["global"];

      parseErrorPages(global["error-pages"], m_errorPages, "");

      if (global.count("client-timeout")) {
        m_clientTimeout = global["client-timeout"];
      }

      if (global.count("server-timeout")) {
        m_serverTimeout = global["server-timeout"];
      }

      if (global.count("nb-threads")) {
        m_threadNr = global["nb-threads"];
      } else {
        m_threadNr = std::thread::hardware_concurrency();
      }

#ifdef ENABLE_LOCAL_CACHE
      m_maxCacheSize = ((float)global["max-cache-size"]) * mbToBytesFactor();
#endif // ENABLE_LOCAL_CACHE

      if (global.count("max-request-size")) {
        m_maxRequestSize = ((int64_t)global["max-request-size"]) * mbToBytesFactor();
      }

      if (global.count("max-in-mem-request-size")) {
        m_maxInMemRequest = ((int64_t) global["max-in-mem-request-size"]) *
                            mbToBytesFactor();
      }

      // backends section
      if (j.count("backends")) {
        parseBackends(j["backends"]);
      }

      if (j.count("connectors")) {
        parseConnectors(j["connectors"]);
      }

      if (j.count("modules")) {
        parseModules(j["modules"]);
      }

      // servers section
      auto servers = j["servers"];

      for (auto& server : servers) {
        ServerConfig* sc = new ServerConfig(m_errorPages);
        sc->m_listenPort = server["listen-port"];
        sc->m_listenHost = server["listen-host"];

        if (server.count("use-ssl") > 0) {
          sc->m_useSSL = server["use-ssl"];

          if (sc->m_useSSL) {
            sc->m_pemfile = server["pem-file"];
            sc->m_dhfile = server["dh-file"];
          }
        } else {
          sc->m_useSSL = false;
        }

        if (server.count("client-timeout")) {
          sc->m_clientTimeout = server["client-timeout"];
        } else {
          sc->m_clientTimeout = m_clientTimeout;
        }

        if (server.count("server-timeout")) {
          sc->m_serverTimeout = server["server-timeout"];
        } else {
          sc->m_serverTimeout = m_serverTimeout;
        }

        // sites section
        for (auto& site : server["sites"]) {
          ServerConfig::Site* s = new ServerConfig::Site();
          std::string host = site["host"];

          if (isLocalDomain(host)) {
            host = replaceLocalDomain(host);
          }

          s->m_host = host;
          s->m_docroot = parseDocRoot(site["docroot"]);

          for (auto& tryf : site["try-files"] ) {
            s->m_tryFiles.push_back(tryf);
          }

          if (site.count("gzip") > 0) {
            std::string line = site["gzip"];
            std::vector<std::string> exts;
            boost::split(exts, line, boost::is_any_of(","));

            for (auto& e : exts) {
              s->m_gzip.insert(boost::trim_copy(e));
            }

          }

          // Copy global settings to local
          s->m_errorPages.insert(m_errorPages.begin(), m_errorPages.end());

          // Overwrite by local setting
          if (site.count("error-pages")) {
            parseErrorPages(site["error-pages"], s->m_errorPages, s->m_docroot);
          }

          if (site.count("locations") > 0) {
            parseLocations(site["locations"], s);
          }

          if (site.count("redirects") > 0) {
            parseRedirections(site["redirects"], s);
          }

          if (site.count("backend") > 0) {
            if (s->m_connectors.size() > 0) {
              throw ConfigException("Cannot define connectors and backend at the same time");
            }

            if (s->m_modules.size() > 0) {
              throw ConfigException("Cannot define modules and backend at the same time");
            }

            if (m_backends.count(site["backend"]) > 0) {
              s->m_backend = site["backend"];
            } else {
              throw ConfigException("Unknown backend " + site["backend"].get<std::string>());
            }
          }

          sc->m_sites.push_back(s);
          sc->m_globalConfig = this;
        }

        m_serversConfig.push_back(sc);
      }

    } catch (std::exception& e) {
      throw ConfigException(e.what());
    }

  }

  void Config::parseErrorPages(json& j, ErrorPageMap& map,
                               const std::string& appendPath)
  {
    for (auto& page : j) {
      for (json::iterator it = page.begin(); it != page.end(); ++it) {
        map[it.key()] = appendPath + it.value().get<std::string>();
      }
    }
  }


  const std::vector<Config::ServerConfig*> Config::serversConfig() const
  {
    return m_serversConfig;
  }

  uint16_t Config::threads() const
  {
    return m_threadNr;
  }

  uint64_t Config::maxCacheSize() const
  {
    return m_maxCacheSize;
  }

  uint64_t Config::maxRequestSize() const
  {
    return m_maxRequestSize;
  }

  uint64_t Config::maxInMemRequestSize() const
  {
    return m_maxInMemRequest;
  }

  const BackendsMap& Config::backends() const
  {
    return m_backends;
  }

  const ConnectorsMap& Config::connectors() const
  {
    return m_connectors;
  }

  const time_t Config::ServerConfig::Site::getCacheTime(const std::string& path)
  const
  {

    for (auto& kv : m_cacheTime) {
      if (StringUtils::regexMatch(kv.first, path)) {
        return kv.second;
      }
    }

    return -1;
  }

  const std::string Config::ServerConfig::Site::connectorName(
    const std::string& path) const
  {
    for (auto& connector : m_connectors)
      if (StringUtils::regexMatch(connector.first, path)) {
        return connector.second;
      }

    return "";
  }

  const Redirection* Config::ServerConfig::Site::redirection(
    const std::string& path) const
  {

    for (const auto& r : m_redirections) {
      if (r.matches(path)) {
        return &r;
      }
    }

    return nullptr;
  }

  bool Config::isLocalDomain(const std::string& domain)
  {
    return domain.find("127.0.0.1") != std::string::npos
           || domain.find("localhost") != std::string::npos;
  }

  std::string Config::localDomain()
  {
    return "localhost";
  }

  std::string Config::replaceLocalDomain(const std::string& domain)
  {
    std::string r = boost::algorithm::replace_all_copy(domain, "127.0.0.1",
                    "localhost");
    return r;
  }

  std::string Config::parseDocRoot(const std::string& docroot) const
  {
    if (docroot.at(0) == '/') {
      return docroot;
    }

    std::string dr = m_cwd + "/" + docroot;
    boost::replace_all(dr, "///", "/");
    boost::replace_all(dr, "//", "/");
    boost::replace_all(dr, "/./", "/");
    return dr;
  }

  void Config::parseBackends(json& j)
  {
    for (auto& backend : j) {
      std::string backendName = backend["name"];

      if (m_backends.count(backendName) > 0) {
        throw ConfigException("Backend " + backendName + " already defined");
      }

      BackendPtr b = std::make_shared<Backends::Backend>();
      b->name = backend["name"];
      b->host = backend["host"];
      b->port = backend["port"];
      m_backends[backendName] = b;
    }
  }

  void Config::parseConnectors(json& j)
  {
    for (auto& connector : j) {
      std::string connectorName = connector["name"];

      if (m_backends.count(connectorName) > 0
          || m_connectors.count(connectorName) > 0) {
        throw ConfigException("Connector " + connectorName +
                              " already defined");
      }

      ConnectorPtr b = std::make_shared<Backends::Connector>();
      b->name = connector["name"];
      b->host = connector["host"];
      b->port = connector["port"];
      b->type = connector["type"] == "fcgi" ? Backends::Connector::FCGI :
                Backends::Connector::Server;
      m_connectors[connectorName] = b;
    }

  }

  void Config::parseModules(json& j)
  {
    for (auto& module : j) {
      std::string moduleName = module["name"];

      if (m_backends.count(moduleName) > 0
          || m_connectors.count(moduleName) > 0) {
        throw ConfigException("Module " + moduleName +
                              " already defined");
      }

      ModulePtr mod = std::make_shared<Modules::Module>();
      mod->name = moduleName;
      mod->path = module["so-path"];
      m_modules[moduleName] = mod;
    }
  }

  void Config::parseLocations(json& j, ServerConfig::Site* s)
  {
    // Parse locations
    for (auto& loc : j) {
      for (json::iterator it = loc.begin(); it != loc.end(); ++it) {
        if (it.value().is_structured()) {
          std::string type = it.value()["type"];

          if (type == "fcgi" || type == "connector") {
            std::string connectorName = it.value()["name"];

            if (m_connectors.count(connectorName) == 0) {
              throw ConfigException("Unknow connector " + connectorName);
            }

            s->m_connectors[it.key()] = it.value()["name"];
          } else if (type == "cache") {
            time_t seconds = parseTimeUnit(it.value()["expires"]);
            s->m_cacheTime[it.key()] = seconds;
          } else if (type == "module") {
            std::string moduleName = it.value()["name"];

            if (m_modules.count(moduleName) == 0) {
              throw ConfigException("Unknow module " + moduleName);
            }

          }
        } else {
          s->m_locations[it.key()] = it.value().get<uint16_t>();
        }
      }
    }
  }

  void Config::parseRedirections(json& j, ServerConfig::Site* s)
  {

    for (auto& r : j) {
      for (json::iterator it = r.begin(); it != r.end(); ++it) {
        std::string pattern = it.value()["redirection"];
        Redirection::Type type = toType(it.value()["type"]);
        s->m_redirections.push_back({it.key(), pattern, type});
      }
    }

  }

  constexpr uint32_t  Config::mbToBytesFactor()
  {
    return 1024 * 1024;
  }

  time_t Config::parseTimeUnit(const std::string& time)
  {
    std::string timeunit;

    if (time.find("y") != std::string::npos) {
      timeunit += "y";
    }

    if (time.find("d") != std::string::npos) {
      timeunit += "d";
    }

    if (time.find("h") != std::string::npos) {
      timeunit += "h";
    }

    if (time.find("m") != std::string::npos) {
      timeunit += "m";
    }

    if (time.find("s") != std::string::npos) {
      timeunit += "s";
    }

    if (timeunit.length() != 1) {
      throw ConfigException("While parsing time units, only one time unit is allowed at once");
    }

    size_t pos = time.find(timeunit);

    std::string timeStr = time.substr(0, pos);

    int timeI =  boost::lexical_cast<int>(timeStr);

    if (timeunit == "y") {
      return timeI * 3.154e+7;
    } else if (timeunit == "d") {
      return timeI * 86400;
    } else if (timeunit == "h") {
      return timeI * 3600;
    } else if (timeunit  == "m") {
      return timeI * 60;
    } else if ( timeunit == "s") {
      return timeI;
    }

    throw ConfigException("Missing time unit ");
  }

  Redirection::Type Config::toType(const std::string& type)
  {
    if (type == "redirect") {
      return Redirection::Type::Redirect;
    } else if (type == "permanent") {
      return Redirection::Type::Permanent;
    }

    throw ConfigException("Invalid redirection type: " + type);

  }


}

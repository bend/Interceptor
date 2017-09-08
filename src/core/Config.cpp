#include "Config.h"

#include "vars.h"
#include "utils/Logger.h"

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

      // servers section
      auto servers = j["servers"];

      for (auto& server : servers) {
        ServerConfig* sc = new ServerConfig(m_errorPages);
        sc->m_listenPort = server["listen-port"];
        sc->m_listenHost = server["listen-host"];

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
            boost::split(exts, line , boost::is_any_of(","));

            for (auto& e : exts) {
              s->m_gzip.insert(boost::trim_copy(e));
            }

          }

          // Copy global settings to local
          s->m_errorPages.insert(m_errorPages.begin(), m_errorPages.end());

          // Overwrite by local setting
          parseErrorPages(site["error-pages"], s->m_errorPages, s->m_docroot);

          // Parse locations

          for (auto& loc : site["locations"]) {
            for (json::iterator it = loc.begin(); it != loc.end(); ++it) {
              if (it.value().is_structured()) {
                BackendPtr backend = std::make_shared<Backend>();
                backend->name = it.value()["name"];
                backend->host = it.value()["host"];
                backend->port = it.value()["port"];
                s->m_connectors[it.key()] = backend;
              } else {
                s->m_locations[it.key()] = it.value().get<uint16_t>();
              }
            }
          }

          if (site.count("backend") > 0) {
            if (s->m_connectors.size() > 0) {
              throw ConfigException("Cannot define connectors and backend at the same time");
            }

            if (m_backends.count(site["backend"])) {
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

      if (m_backends.count(backendName)) {
        throw ConfigException("Backend " + backendName + " already defined");
      }

      BackendPtr b = std::make_shared<Backend>();
      b->name = backend["name"];
      b->host = backend["host"];
      b->port = backend["port"];
      m_backends[backendName] = b;
    }
  }

  constexpr uint32_t  Config::mbToBytesFactor()
  {
    return 1024 * 1024;
  }

}

#include "Config.h"

#include "Logger.h"

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <thread>


Config::Config(const std::string& path)
  : m_path(path),
    m_clientTimeout(0),
    m_serverTimeout(0)
{
  parse();
}

void Config::parse()
{

  trace("info") << "Reading configuration file " << m_path;

  std::ifstream config(m_path);

  if (!config.is_open()) {
    throw ConfigException("Cannot open config file " + m_path);
  }

  try {
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

      for (auto& site : server["sites"]) {
        ServerConfig::Site* s = new ServerConfig::Site();
        s->m_host = site["host"];
        s->m_docroot = site["docroot"];

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

        sc->m_sites.push_back(s);
      }

      m_serversConfig.push_back(sc);
    }

  } catch (std::exception& e) {
    throw ConfigException(e.what());
  }

}

void Config::parseErrorPages(json& j, ErrorPageMap& map, const std::string appendPath)
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


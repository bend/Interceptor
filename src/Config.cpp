#include "Config.h"

#include "Logger.h"

#include <fstream>

Config::Config(const std::string& path)
  : m_path(path)
{
  parse();
}

void Config::parse()
{

  trace("info") << "Reading configuration file " << m_path;

  std::ifstream config(m_path);

  if (!config.is_open())
    throw ConfigException("Cannot open config file " + m_path);

  try {
    json j = json::parse(config);

    auto global = j["global"];

    parseErrorPages(global["error-pages"], m_errorPages, "");

    auto servers = j["servers"];

    for (auto& server : servers) {
      ServerConfig* sc = new ServerConfig(m_errorPages);
      sc->m_listenPort = server["listen-port"];
      sc->m_listenHost = server["listen-host"];

      for (auto& site : server["sites"]) {
        ServerConfig::Site* s = new ServerConfig::Site();
        s->m_host = site["host"];
        s->m_docroot = site["docroot"];

        for (auto& tryf : site["try-files"] ) {
          s->m_tryFiles.push_back(tryf);
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

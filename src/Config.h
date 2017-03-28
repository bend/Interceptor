#ifndef CONFIG_H__
#define CONFIG_H__

#include <string>
#include <vector>
#include <map>
#include "json/json.hpp"

using json = nlohmann::json;

typedef std::map<std::string, std::string> ErrorPageMap;

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
	  ErrorPageMap m_errorPages;
    };

    std::string m_listenHost;
    int m_listenPort;

    std::vector<Site*> m_sites;
	const ErrorPageMap& m_errorPages;

  };

public:
  Config(const std::string& path);

  const std::vector<ServerConfig*> serversConfig() const;

private:
  void parse();
  void parseErrorPages(json& j, ErrorPageMap& map, const std::string appendPath);
private:
  const std::string m_path;
  std::vector<ServerConfig*> m_serversConfig;
  ErrorPageMap m_errorPages;


};

#endif //CONFIG_H__

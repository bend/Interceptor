#include "ServerInfo.h"

#include "global.h"

#include <sstream>
#include <iomanip>

namespace Interceptor::ServerInfo {

  std::string commonName()
  {
    return name() + "/" + version() + " (" + osName() + ")";
  }

  std::string version()
  {
    return std::to_string(SERVER_VERSION_MAJOR) + "." + std::to_string(
             SERVER_VERSION_MINOR) + "." + std::to_string(SERVER_VERSION_PATCH);
  }

  std::string name()
  {
    return SERVER_NAME;
  }

  std::string osName()
  {
#ifdef _WIN32
    return "Windows 32-bit";
#elif _WIN64
    return "Windows 64-bit";
#elif __unix || __unix__
    return "Unix";
#elif __APPLE__ || __MACH__
    return "Mac OSX";
#elif __linux__
    return "Linux";
#elif __FreeBSD__
    return "FreeBSD";
#else
    return "Other";
#endif
  }

  std::string build()
  {
    return INTERCEPTOR_GIT_COMMIT_ID;
  }

  std::string buildDate()
  {
    return INTERCEPTOR_BUILD_DATE;
  }

  std::string currentDate()
  {
    std::time_t t = std::time(nullptr);
    std::stringstream sstr;
    sstr << std::put_time(std::gmtime(&t), "%a, %d %b %Y %T %Z");
    return sstr.str();
  }

}

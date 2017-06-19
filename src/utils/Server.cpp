#include "Server.h"

namespace Http {

  namespace Server {

    std::string getCommonName()
    {
      return getName() + "/" + getVersion() + " (" + getOsName() + ")";
    }

    std::string getVersion()
    {
      return SERVER_VERSION;
    }

    std::string getName()
    {
      return SERVER_NAME;
    }

    std::string getOsName()
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

    std::string getBuild()
    {
      return INTERCEPTOR_GIT_COMMIT_ID;
    }

  }

}

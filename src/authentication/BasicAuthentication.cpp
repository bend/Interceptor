#include "BasicAuthentication.h"

#include "Authentication.h"
#include "http/Request.h"

#include <boost/algorithm/string.hpp>

namespace Interceptor::Authentication {

  BasicAuthentication::BasicAuthentication(AuthenticationCPtr config)
    : AbstractAuthentication(config)
  {}

  bool BasicAuthentication::authenticate(HttpRequestPtr request) const
  {
    const std::string* header = request->getHeader("Authorization");

    if (!header) {
      return false;
    }

    std::vector<std::string> parts;
    boost::split(parts, *header, boost::is_any_of(" "));

    if (parts[0] == "Basic")  {
      if (parts[1] == m_config->credentials) {
        return true;
      }
    }

    return false;
  }

}

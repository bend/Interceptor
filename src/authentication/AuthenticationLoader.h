#ifndef AUTHENTICATION_LOADER_H__
#define AUTHENTICATION_LOADER_H__

#include "defs.h"

#include <vector>
#include <map>

namespace Interceptor::Authentication {

  class AuthenticationLoader {
  public:
    AuthenticationLoader() = default;


    bool loadAuthentications(std::vector<AuthenticationCPtr>& auths);
    bool loadAuthentication(AuthenticationCPtr auth);

    AbstractAuthenticationPtr get(const std::string& name) const;

  private:
    std::map<std::string, AbstractAuthenticationPtr> m_auths;

  };

}

#endif // AUTHENTICATION_LOADER_H__

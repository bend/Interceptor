#include "AuthenticationLoader.h"

#include "Authentication.h"
#include "BasicAuthentication.h"


namespace Interceptor::Authentication {

  bool AuthenticationLoader::loadAuthentications(std::vector<AuthenticationCPtr>&
      auths)
  {
    for (auto& a : auths) {
      if (!loadAuthentication(a)) {
        return false;
      }
    }

    return true;
  }

  bool AuthenticationLoader::loadAuthentication(AuthenticationCPtr auth)
  {
    m_auths[auth->name] = std::make_shared<BasicAuthentication>(auth);
    return true;
  }

  AbstractAuthenticationPtr AuthenticationLoader::get(const std::string& name)
  const
  {
    if (m_auths.count(name) > 0) {
      return m_auths.at(name);
    }

    return nullptr;
  }
}

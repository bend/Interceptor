#ifndef AUTHENTICATION_DEFS_H__
#define AUTHENTICATION_DEFS_H__

#include <memory>

namespace Interceptor {
  namespace Authentication {
    class Authentication;
    class AbstractAuthentication;
    class AuthenticationLoader;
  }
  typedef std::shared_ptr<const Authentication::Authentication>
  AuthenticationCPtr;
  typedef std::shared_ptr<Authentication::Authentication> AuthenticationPtr;
  typedef std::shared_ptr<Authentication::AbstractAuthentication>
  AbstractAuthenticationPtr;

}

#endif // AUTHENTICATION_DEFS_H__

#ifndef ABSTRACT_AUTHENTICATION_H__
#define ABSTRACT_AUTHENTICATION_H__

#include "common/Defs.h"
#include "Authentication.h"

namespace Interceptor::Authentication {

class AbstractAuthentication {
  public:
	AbstractAuthentication(AuthenticationCPtr config);
	virtual ~AbstractAuthentication() = default;
	virtual bool authenticate(HttpRequestPtr request) const = 0;

  protected:
	AuthenticationCPtr m_config;
};
    
typedef std::shared_ptr<AbstractAuthentication> AbstractAuthenticationPtr;

}

#endif // ABSTRACT_AUTHENTICATION_H__

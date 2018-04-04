#ifndef ABSTRACT_AUTHENTICATION_H__
#define ABSTRACT_AUTHENTICATION_H__

#include "common/Defs.h"

namespace Interceptor::Authentication {

class AbstractAuthentication {
  public:
	virtual ~AbstractAuthentication() = default;
	virtual bool authenticate(HttpRequestPtr request) const = 0;
};
    
typedef std::shared_ptr<AbstractAuthentication> AbstractAuthenticationPtr;

}

#endif // ABSTRACT_AUTHENTICATION_H__

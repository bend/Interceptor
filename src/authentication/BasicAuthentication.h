#ifndef BASIC_AUTHENTICATION_H__
#define BASIC_AUTHENTICATION_H__

#include "AbstractAuthentication.h"

namespace Interceptor::Authentication {


class BasicAuthentication : public AbstractAuthentication {
  public:
	BasicAuthentication(AuthenticationCPtr config);
	virtual ~BasicAuthentication() = default;
	virtual bool authenticate(HttpRequestPtr request) const;

};

}

#endif // BASIC_AUTHENTICATION_H__

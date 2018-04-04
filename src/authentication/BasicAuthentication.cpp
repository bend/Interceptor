#include "BasicAuthentication.h"

namespace Interceptor::Authentication {
  
  bool BasicAuthentication::authenticate(HttpRequestPtr request) const
  {
	return true;
  }

}

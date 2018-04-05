#include "AbstractAuthentication.h"

namespace Interceptor::Authentication {

  AbstractAuthentication::AbstractAuthentication(AuthenticationCPtr config):
	m_config(config)
  {}

}

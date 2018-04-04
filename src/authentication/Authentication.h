#ifndef AUTHENTICATION_H__
#define AUTHENTICATION_H__

#include <string>
#include <memory>

namespace Interceptor{
  namespace Authentication {

  struct Authentication {

	enum Type  {
	  Basic
	};

	std::string name;
	std::string credentials;
	Type type;

  };
	
  }

  typedef std::shared_ptr<const Authentication::Authentication> AuthenticationCPtr;
  typedef std::shared_ptr<Authentication::Authentication> AuthenticationPtr;

}

#endif // AUTHENTICATION_H__

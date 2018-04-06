#ifndef AUTHENTICATION_H__
#define AUTHENTICATION_H__

#include <string>
#include <memory>

namespace Interceptor {
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

}

#endif // AUTHENTICATION_H__

#ifndef MODULE_H__
#define MODULE_H__

#include <string>
#include <memory>

namespace Interceptor {
  namespace Modules {

    struct Module {
      std::string name;
      std::string path;
      bool loadOnStart;
    };
  }
}
#endif // MODULE_H__

#ifndef MODULES_DEFS_H__
#define MODULES_DEFS_H__

#include <memory>

namespace Interceptor {
  namespace Modules {

    class AbstractModule;
    struct Module;

  }

  typedef Modules::AbstractModule* AbstractModulePtr;
  typedef std::shared_ptr<const Modules::Module> ModuleCPtr;
  typedef std::shared_ptr<Modules::Module> ModulePtr;

}

#endif // MODULES_DEFS_H__

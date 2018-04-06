#ifndef MODULES_LOADER_H__
#define MODULES_LOADER_H__

#include "Module.h"

#include "defs.h"

#include <vector>
#include <unordered_map>

namespace Interceptor::Modules {

  class ModulesLoader {

  public:
    ModulesLoader() = default;
    ~ModulesLoader();

    bool loadModules(std::vector<ModuleCPtr>& modules);
    bool loadModule(ModuleCPtr module);

    AbstractModulePtr get(const std::string& moduleName) const;

  private:
    std::unordered_map<std::string, AbstractModulePtr> m_modules;


  };



}

#endif // MODULES_LOADER_H__

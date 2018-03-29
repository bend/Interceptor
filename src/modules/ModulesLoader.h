#ifndef MODULES_LOADER_H__
#define MODULES_LOADER_H__

#include "Module.h"

#include <vector>
#include <unordered_map>

namespace Interceptor::Modules {

  class AbstractModule;

  class ModulesLoader {

  public:
    ModulesLoader() = default;
    ~ModulesLoader();

    bool loadModules(std::vector<ModuleCPtr>& modules);
    bool loadModule(ModuleCPtr module);

    AbstractModule* get(const std::string& moduleName) const;

  private:
    std::unordered_map<std::string, AbstractModule*> m_modules;


  };



}

#endif // MODULES_LOADER_H__

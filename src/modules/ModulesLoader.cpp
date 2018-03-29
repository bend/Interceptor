#include "ModulesLoader.h"

#include <Module.h>
#include <AbstractModule.h>
#include <utils/Logger.h>


#include <dlfcn.h>

namespace Interceptor::Modules {

  ModulesLoader::~ModulesLoader()
  {
    for (auto kv : m_modules) {
      dlclose(kv.second->m_handler);
    }
  }

  AbstractModule* ModulesLoader::get(const std::string& moduleName) const
  {
    if (m_modules.count(moduleName) > 0) {
      return m_modules.at(moduleName);
    }

    return nullptr;
  }


  bool ModulesLoader::loadModules(std::vector<ModuleCPtr>& modules)
  {
    bool ret = true;

    for (auto& m : modules) {
      ret = ret && loadModule(m);
    }

    return ret;
  }

  bool ModulesLoader::loadModule(ModuleCPtr module)
  {
    void* handler = dlopen(module->path.c_str(), RTLD_LAZY);

    if (!handler) {
      LOG_ERROR("ModulesLoader::loadModule() - Could not load module : " <<
                dlerror());
      return false;
    }

    pf_Module func = (pf_Module) dlsym(handler, "make");

    AbstractModule* pModule = func();
    pModule->m_handler = handler;

    m_modules[module->name] = pModule;

    return true;
  }

}

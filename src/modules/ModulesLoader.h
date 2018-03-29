#ifndef MODULES_LOADER_H__
#define MODULES_LOADER_H__

#include "Module.h"

namespace Interceptor::Modules {

  class ModulesLoader {

  public:
    ModulesLoader() = default;

    bool loadModule(const Module& module);


  };



}

#endif // MODULES_LOADER_H__

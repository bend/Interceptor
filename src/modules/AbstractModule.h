#ifndef ABSTRACT_MODULE_H__
#define ABSTRACT_MODULE_H__

#include "http/Http.h"

#include "common/Defs.h"

#include <sstream>
#include <functional>

namespace Interceptor::Modules {

  class AbstractModule {
  public:

    virtual void handleRequest(
      std::function<void(BufferPtr)> callback) = 0;

  private:
    void* m_handler;

    friend class ModulesLoader;
  };

  typedef AbstractModule* (*pf_Module)();
}

#endif

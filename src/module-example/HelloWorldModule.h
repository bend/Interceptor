#ifndef HELLO_WORLD_MODULE_H__

#include "modules/AbstractModule.h"
#include "common/defs.h"

class HelloWorld : public Interceptor::Modules::AbstractModule {
public:
  HelloWorld();
  virtual void handleRequest(
    std::function<void(Interceptor::BufferPtr)> callback);
};

extern "C" {

  HelloWorld* make()
  {
    return new HelloWorld();
  }

}

#endif //HELLO_WORLD_MODULE

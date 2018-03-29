#ifndef EVENT_H__
#define EVENT_H__

#include <string>

namespace Interceptor::Cache {

  struct Event {
    int event;
    std::string path;

  };

}

#endif // EVENT_H__

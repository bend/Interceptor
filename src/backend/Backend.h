#ifndef BACKEND_H__
#define BACKEND_H__

#include <string>
#include <memory>

struct Backend {
  std::string name;
  std::string host;
  uint16_t port;
};

typedef std::shared_ptr<const Backend> BackendCPtr;
typedef std::shared_ptr<Backend> BackendPtr;

#endif // BACKEND_H__

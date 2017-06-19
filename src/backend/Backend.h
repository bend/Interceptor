#ifndef BACKEND_H__
#define BACKEND_H__

#include <string>

struct Backend {
  std::string name;
  std::string host;
  uint16_t port;
};

#endif // BACKEND_H__

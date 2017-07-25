#ifndef ABSTRACT_CONNECTOR_H__
#define ABSTRACT_CONNECTOR_H__

#include <http/Http.h>
#include <memory>


class AbstractConnector {
public:
  virtual ~AbstractConnector() = default;
  virtual bool connect() = 0;
  virtual void forward(const char* data, size_t size,
                       std::function<void(Http::Code)> callback) = 0;
  virtual const std::string& name() const = 0;

};

typedef std::shared_ptr<AbstractConnector> AbstractConnectorPtr;

#endif

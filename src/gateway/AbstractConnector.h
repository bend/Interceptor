#ifndef ABSTRACT_CONNECTOR_H__
#define ABSTRACT_CONNECTOR_H__

#include <http/Http.h>
#include "common/Defs.h"
#include <memory>
#include <sstream>


class AbstractConnector {
public:
  virtual ~AbstractConnector() = default;
  virtual bool connect() = 0;
  virtual void forward(Packet& packet,
                       std::function<void(Http::Code)> callback) = 0;
  virtual const std::string& name() const = 0;
  virtual void reset() = 0;
  virtual void readReply(std::function<void(Http::Code, std::stringstream*)> callback) = 0;

};

typedef std::shared_ptr<AbstractConnector> AbstractConnectorPtr;

#endif

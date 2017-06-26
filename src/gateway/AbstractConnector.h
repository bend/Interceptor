#ifndef ABSTRACT_CONNECTOR_H__
#define ABSTRACT_CONNECTOR_H__

#include <memory>


class AbstractConnector {
public:
  virtual ~AbstractConnector() = default;
  virtual bool connect() = 0;
  virtual const std::string& name() const = 0;

};

typedef std::unique_ptr<AbstractConnector> AbstractConnectorPtr;

#endif

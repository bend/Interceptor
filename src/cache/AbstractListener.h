#ifndef ABSTRACT_LISTENER_H__
#define ABSTRACT_LISTENER_H__

#include "Event.h"

class AbstractListener {
public:
  virtual ~AbstractListener() = default;

  virtual void notify(Event& e) = 0;

};

#endif // ABSTRACT_LISTENER_H__

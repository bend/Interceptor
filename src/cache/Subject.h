#ifndef SUBJECT_H__
#define SUBJECT_H__

#include <list>

#include "Event.h"

class AbstractListener;

class Subject {

public:
  Subject() = default;

  void addListener(AbstractListener* listener);

  void notifyListeners(Event e);

private:
  std::list<AbstractListener*> m_listeners;

};

#endif // SUBJECT_H__

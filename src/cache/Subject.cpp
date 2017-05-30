#include "Subject.h"

#include "AbstractListener.h"

void Subject::addListener(AbstractListener* listener)
{
  m_listeners.push_back(listener);
}

void Subject::notifyListeners(Event e)
{
  for (auto& listener : m_listeners) {
    listener->notify(e);
  }
}

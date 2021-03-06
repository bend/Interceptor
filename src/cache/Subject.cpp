#include "Subject.h"

#include "AbstractListener.h"

namespace Interceptor::Cache {

  Subject::~Subject()
  {
    for (auto& listener : m_listeners) {
      delete listener;
    }

    m_listeners.clear();
  }

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

}

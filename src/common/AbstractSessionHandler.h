#ifndef ABSTRACT_SESSION_HANDLER_H__
#define ABSTRACT_SESSION_HANDLER_H__

#include "Defs.h"

namespace Interceptor {
  class AbstractSessionHandler {

  public:
    AbstractSessionHandler(SessionConnectionPtr connection);
    AbstractSessionHandler(AbstractSessionHandler&) = delete;
    ~AbstractSessionHandler() = default;

    virtual void transferSession(const char* data, size_t bytes) = 0;

  protected:
    SessionConnectionPtr m_connection;

  };

}

#endif // ABSTRACT_SESSION_HANDLER_H__

#include "AbstractSessionHandler.h"

namespace Interceptor {

  AbstractSessionHandler::AbstractSessionHandler(SessionConnectionPtr connection)
    : m_connection(connection)
  {
  }

};



#ifndef CORE_DEFS_H__
#define CORE_DEFS_H__

#include <memory>

namespace Interceptor {

  class Session;
  class SessionConnection;

  typedef std::shared_ptr<Session> SessionPtr;
  typedef std::weak_ptr<Session> SessionWeakPtr;
  typedef std::shared_ptr<SessionConnection> SessionConnectionPtr;

  typedef std::string Host;

}

#endif // CORE_DEFS_H__

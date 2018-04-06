#ifndef SOCKET_DEFS_H__
#define SOCKET_DEFS_H__

#include <memory>

namespace Interceptor {
  namespace Network {

    class InboundConnection;
    class OutboundConnection;

  }

  typedef std::shared_ptr<Network::InboundConnection> InboundConnectionPtr;
  typedef std::shared_ptr<Network::OutboundConnection> OutboundConnectionPtr;

}

#endif // SOCKET_DEFS_H__

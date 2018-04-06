#ifndef BACKEND_GATEWAY_H__
#define BACKEND_GATEWAY_H__

#include "AbstractGateway.h"

#include <boost/signals2/signal.hpp>

namespace Interceptor {

  struct Packet;

  namespace Backends {

    class BackendGateway : public AbstractGateway,
      public std::enable_shared_from_this<BackendGateway> {
    public:
      BackendGateway(HttpRequestPtr request);

      virtual ~BackendGateway();

      virtual void setConnection(AbstractConnectorPtr connection) override;

      virtual AbstractConnectorPtr takeConnection() override;

      virtual void handleRequest(
        std::function<void(Http::StatusCode, std::stringstream*)>&
        func) override;

      virtual void reset() override;

    private:
      void forward(Packet* packet);
      void hasMoreData();

    private:
      AbstractConnectorPtr m_connection;
      boost::signals2::scoped_connection m_signalCon;
      std::function<void(Http::StatusCode, std::stringstream*)> m_callback;
    };

  }
}

#endif // BACKEND_GATEWAY_H__

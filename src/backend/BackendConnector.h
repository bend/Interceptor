#ifndef BACKEND_CONNECTOR_H__
#define BACKEND_CONNECTOR_H__

#include "gateway/AbstractConnector.h"
#include "defs.h"
#include "socket/defs.h"

#include <memory>
#include <deque>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>

namespace Interceptor {

  struct Packet;

  namespace Backends {

    class BackendConnector : public AbstractConnector,
      public std::enable_shared_from_this<BackendConnector> {

    public:
      BackendConnector(BackendCPtr backend, boost::asio::io_service& ioService);
      virtual ~BackendConnector();
      virtual bool connect() override;
      virtual void forward(Packet* packet,
                           std::function<void(Http::StatusCode)> callback) override;
      virtual const std::string& name() const override;
      virtual void reset() override;
      virtual void setReplyCallback(
        std::function<void(Http::StatusCode, std::stringstream*)>& callback) override;

    protected:
      virtual void readReply(std::function<void(Http::StatusCode, std::stringstream*)>&
                             callback) override;

    private:
      void handleResolved(const boost::system::error_code& error,
                          boost::asio::ip::tcp::resolver::iterator it);
      void handleConnected(const boost::system::error_code& error);
      void handleResponseRead(const boost::system::error_code& error,
                              size_t bytesRead, std::function<void(Http::StatusCode, std::stringstream*)>&
                              callback);
      void handlePacketForwarded(const boost::system::error_code& error,
                                 Packet*  packet,
                                 std::function<void(Http::StatusCode)>& callback);
      void doPost(std::pair<Packet*, std::function<void(Http::StatusCode)>>& data);
      void forwardNext();
      void doForward(Packet* packet, std::function<void(Http::StatusCode)>& callback);

    private:
      enum State {
        CanWrite = 0x01,
        Closing = 0x02,
        Connected = 0x04,
        Reading = 0x08,
        Writing = 0x10
      };

    private:
      BackendCPtr m_backend;
      boost::asio::io_service& m_ioService;
      OutboundConnectionPtr m_connection;
      char m_response[4096];
      std::deque<std::pair<Packet*, std::function<void(Http::StatusCode)>>> m_outbox;
      boost::asio::strand m_writestrand;
      boost::asio::strand m_readstrand;
      boost::asio::strand m_callbackstrand;
      int m_state;
      std::function<void(Http::StatusCode, std::stringstream*)> m_callback;
    };

  }

}

#endif // OUTBOUND_CONNECTION_H__

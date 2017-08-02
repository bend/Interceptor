#ifndef BACKEND_CONNECTOR_H__
#define BACKEND_CONNECTOR_H__

#include "Backend.h"
#include "gateway/AbstractConnector.h"
#include <memory>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>
#include <deque>

class OutboundConnection;

class BackendConnector : public AbstractConnector,
  public std::enable_shared_from_this<BackendConnector> {

public:
  BackendConnector(BackendCPtr backend, boost::asio::io_service& ioService);
  virtual ~BackendConnector();
  virtual bool connect() override;
  virtual void forward(Packet& packet,
                       std::function<void(Http::Code)> callback) override;
  virtual const std::string& name() const override;
  virtual void reset() override;

private:
  void handleResolved(const boost::system::error_code& error,
                      boost::asio::ip::tcp::resolver::iterator it);
  void handleConnected(const boost::system::error_code& error);
  void handleResponseRead(const boost::system::error_code& error,
                          size_t bytesRead, std::function<void(Http::Code, std::stringstream&)> callback);
  void handlePacketForwarded(const boost::system::error_code& error, std::function<void(Http::Code)> callback);
  void doPost(std::pair<Packet, std::function<void(Http::Code)>> data);
  void forwardNext();
  void doForward(Packet& packet, std::function<void(Http::Code)> callback);

private:
  enum State {
	CanWrite = 0x01,
	Closing = 0x02,
	Connected = 0x04
  };

private:
  BackendCPtr m_backend;
  boost::asio::io_service& m_ioService;
  std::shared_ptr<OutboundConnection> m_connection;
  char m_response[4096];
  std::deque<std::pair<Packet, std::function<void(Http::Code)>>> m_outbox;
  boost::asio::strand m_strand;
  int m_state;
};

#endif // OUTBOUND_CONNECTION_H__

#ifndef BACKEND_CONNECTOR_H__
#define BACKEND_CONNECTOR_H__

#include "Backend.h"
#include "gateway/AbstractConnector.h"
#include <memory>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>

class OutboundConnection;

class BackendConnector : public AbstractConnector,
  public std::enable_shared_from_this<BackendConnector> {

public:
  BackendConnector(BackendCPtr backend, boost::asio::io_service& ioService);
  ~BackendConnector();
  virtual bool connect() override;
  virtual void forward(const char* data, size_t size, std::function<void(Http::Code)> callback) override;
  virtual const std::string& name() const override;

private:
  void handleResolved(const boost::system::error_code& error,
                      boost::asio::ip::tcp::resolver::iterator it);
  void handleConnected(const boost::system::error_code& error);
  void handleResponseRead(const boost::system::error_code& error, size_t bytesRead, std::function<void(Http::Code, std::stringstream&)> callback);


private:
  BackendCPtr m_backend;
  boost::asio::io_service& m_ioService;
  std::shared_ptr<OutboundConnection> m_connection;
  char m_response[4096];
};

#endif // OUTBOUND_CONNECTION_H__

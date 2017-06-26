#ifndef BACKEND_CONNECTOR_H__
#define BACKEND_CONNECTOR_H__

#include "Backend.h"
#include "gateway/AbstractConnector.h"
#include <memory>
#include <boost/asio/io_service.hpp>

class OutboundConnection;

class BackendConnector : public AbstractConnector {

public:
  BackendConnector(BackendCPtr backend, boost::asio::io_service& ioService);
  ~BackendConnector();
  virtual bool connect() override;
  virtual const std::string& name() const override;

private:
  BackendCPtr m_backend;
  boost::asio::io_service& m_ioService;
  std::shared_ptr<OutboundConnection> m_connection;

};

#endif // OUTBOUND_CONNECTION_H__

#ifndef BACKEND_CONNECTOR_H__
#define BACKEND_CONNECTOR_H__

#include "Backend.h"

#include <memory>
#include <boost/asio/io_service.hpp>

class OutboundConnection;

class BackendConnector {

public:
  BackendConnector(const Backend& backend, boost::asio::io_service& ioService);
  bool connect();

private:
  const Backend& m_backend;
  boost::asio::io_service& m_ioService;
  std::shared_ptr<OutboundConnection> m_connection;

};

#endif // OUTBOUND_CONNECTION_H__

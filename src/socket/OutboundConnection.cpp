#include "OutboundConnection.h"
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>

OutboundConnection::OutboundConnection(boost::asio::io_service& io_service,
                                       const std::string& host, const std::string& port):
  resolver_(io_service),
  host_(host),
  port_(port),
  io_service_(io_service)
{
}

void OutboundConnection::asyncResolve(
  boost::function2<void, boost::system::error_code, tcp::resolver::iterator>
  callback)
{
  resolver_.async_resolve(tcp::resolver::query(host_, port_), callback);
}

void OutboundConnection::setEndpoint(tcp::resolver::iterator endpoint)
{
  endpointIterator_ = endpoint;
}

OutboundTcpConnection::OutboundTcpConnection(boost::asio::io_service&
    io_service,
    const std::string& host, const std::string& port ) :
  OutboundConnection(io_service, host, port),
  m_spSocket(new tcp::socket(io_service))
{
}

void OutboundTcpConnection::asyncRead( void* b, size_t size,
                                       boost::function2<void, boost::system::error_code, size_t> callback)
{
  async_read(*m_spSocket,
             boost::asio::buffer(b, size),
             boost::asio::transfer_at_least(size),
             callback);
}

void OutboundTcpConnection::asyncWrite( const void* data, size_t size,
                                        boost::function2<void, boost::system::error_code,
                                        size_t> callback)
{
  async_write(*m_spSocket, boost::asio::buffer(data, size), callback);
}

boost::system::error_code OutboundTcpConnection::write(const void* data,
    size_t size)
{
  boost::system::error_code ec;
  boost::asio::write(*m_spSocket, boost::asio::buffer(data, size), ec);
  return ec;
}

void OutboundTcpConnection::disconnect()
{
  try {
    boost::system::error_code ec;
    m_spSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    m_spSocket->cancel();
    m_spSocket->close();
  } catch (std::exception& e) {
  }
}

void OutboundTcpConnection::asyncConnect(
  boost::function1<void, boost::system::error_code> callback)
{
  m_spSocket.reset(new tcp::socket(io_service_));
  boost::asio::async_connect(*m_spSocket , endpointIterator_,
                             boost::bind(callback,
                                         boost::asio::placeholders::error));
}

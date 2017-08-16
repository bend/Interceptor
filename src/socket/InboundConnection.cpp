#include "InboundConnection.h"

#include <boost/lexical_cast.hpp>
#include "utils/Logger.h"

namespace Interceptor {

  std::string InboundConnection::ip()
  {
    try {
      return socket().remote_endpoint().address().to_string()
             + ":"
             + boost::lexical_cast<std::string>(socket().remote_endpoint().port());
    } catch (...) {
      return "";
    }
  }

  TcpInboundConnection::TcpInboundConnection(boost::asio::io_service& io_service)
    :
    m_spSocket(io_service)
  {
  }

  void TcpInboundConnection::asyncRead( void* b, size_t size,
                                        std::function<void(boost::system::error_code, size_t)> callback)
  {
    async_read(m_spSocket, boost::asio::buffer(b, size),
               boost::asio::transfer_at_least(size),
               callback);
  }

  void TcpInboundConnection::asyncReadSome(void* data, size_t size,
      std::function<void(boost::system::error_code, size_t)> callback)
  {
    m_spSocket.async_read_some(boost::asio::buffer(data, size),
                               callback);
  }

  void TcpInboundConnection::asyncWrite( const void* data, size_t size,
                                         std::function<void(boost::system::error_code,
                                             size_t)> callback)
  {
    async_write(m_spSocket, boost::asio::buffer(data, size), callback);
  }

  void TcpInboundConnection::asyncWrite(const
                                        std::vector<boost::asio::const_buffer>& buffers,
                                        std::function<void(boost::system::error_code,
                                            size_t)> callback)
  {
    async_write(m_spSocket, buffers, callback);
  }

  void TcpInboundConnection::disconnect()
  {
    try {
      LOG_DEBUG( "TcpInboundConnection::disconnect() - closing connection with " << ip());
      boost::system::error_code ec;
      m_spSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
      m_spSocket.cancel();
      m_spSocket.close();
    } catch (std::exception& e) {
    }
  }

}

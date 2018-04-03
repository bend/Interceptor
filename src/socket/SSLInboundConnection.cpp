#include "SSLInboundConnection.h"

namespace Interceptor::Network {

  SSLInboundConnection::SSLInboundConnection(boost::asio::io_service& io_service,
      boost::asio::ssl::context& context) :
    m_spSocket(io_service, context)
  {
#ifdef SSL_OP_NO_COMPRESSION
    SSL_set_options(m_spSocket.native_handle(), SSL_OP_NO_COMPRESSION);
#endif
  }

  void SSLInboundConnection::asyncRead( void* b, size_t size,
                                        std::function<void(boost::system::error_code, size_t)> callback)
  {
    async_read(m_spSocket, boost::asio::buffer(b, size),
               boost::asio::transfer_at_least(size),
               callback);
  }

  void SSLInboundConnection::asyncWrite( const void* data, size_t size,
                                         std::function<void(boost::system::error_code, size_t)> callback)
  {
    async_write(m_spSocket, boost::asio::buffer(data, size), callback);
  }

  void SSLInboundConnection::asyncWrite(const
                                        std::vector<boost::asio::const_buffer>& buffers,
                                        std::function<void(boost::system::error_code,
                                            size_t)> callback)
  {
    async_write(m_spSocket, buffers, callback);
  }

  void SSLInboundConnection::asyncReadSome(void* data, size_t size,
      std::function<void(boost::system::error_code, size_t)> callback)
  {
    m_spSocket.async_read_some(boost::asio::buffer(data, size),
                               callback);
  }

  void SSLInboundConnection::asyncHandshake(
    std::function<void(boost::system::error_code)> callback)
  {
    m_spSocket.async_handshake( boost::asio::ssl::stream_base::server, callback);
  }

  void SSLInboundConnection::disconnect()
  {
    try {
      boost::system::error_code ec;
      socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
      m_spSocket.next_layer().shutdown( boost::asio::ip::tcp::socket::shutdown_both,
                                        ec );
      m_spSocket.lowest_layer().close();
      socket().cancel();
      socket().close();
    } catch (std::exception& e) {
    }
  }

}

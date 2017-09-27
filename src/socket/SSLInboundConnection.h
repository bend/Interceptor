#ifndef SSL_INBOUND_CONNECTION_H__
#define SSL_INBOUND_CONNECTION_H__

#include "InboundConnection.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace Interceptor {

  class SSLInboundConnection : public InboundConnection {
  public:

    SSLInboundConnection(boost::asio::io_service& io_service,
                         boost::asio::ssl::context& context);


    virtual void asyncRead( void* data, size_t size,
                            std::function<void(boost::system::error_code, size_t)> callback) override;

    virtual void asyncWrite( const void* data, size_t size,
                             std::function<void(boost::system::error_code,
                                 size_t)> callback) override;

    virtual void asyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                            std::function<void(boost::system::error_code,
                                size_t)> callback) override;

    virtual void asyncReadSome(void* data, size_t size,
                               std::function<void(boost::system::error_code, size_t)> callback);


    void asyncHandshake(std::function<void(boost::system::error_code)> callback);

    virtual void disconnect() override;

    virtual tcp::socket& socket() override
    {
      return m_spSocket.next_layer();
    }

  protected:
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

    ssl_socket m_spSocket;

  };

}

#endif // SSL_INBOUND_CONNECTION_H__

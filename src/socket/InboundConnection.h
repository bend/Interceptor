#ifndef TCP_CONNECTION_H__
#define TCP_CONNECTION_H__

#include <boost/asio.hpp>
#include <boost/regex.hpp>

using boost::asio::ip::tcp;

namespace Interceptor {

  class InboundConnection {
  public:

    virtual ~InboundConnection() = default;

    virtual void asyncRead( void* data, size_t size,
                            std::function<void(boost::system::error_code, size_t)> callback) = 0;

    virtual void asyncWrite( const void* data, size_t size,
                             std::function<void(boost::system::error_code,
                                 size_t)> callback) = 0;

    virtual void asyncWrite( const std::vector<boost::asio::const_buffer>& buffers,
                             std::function<void(boost::system::error_code,
                                 size_t)> callback) = 0;

    virtual void asyncReadSome(void* data, size_t size,
                               std::function<void(boost::system::error_code, size_t)> callback) = 0;

    virtual void disconnect() = 0;

    virtual tcp::socket& socket() = 0;

    virtual std::string ip();
  };

  class TcpInboundConnection : public InboundConnection {

  public:
    virtual ~TcpInboundConnection() = default;

    TcpInboundConnection(boost::asio::io_service& io_service);
    virtual void asyncRead( void* data, size_t size,
                            std::function<void(boost::system::error_code, size_t)> callback) override;

    virtual void asyncReadSome(void* data, size_t size,
                               std::function<void(boost::system::error_code, size_t)> callback) override;

    virtual void asyncWrite( const void* data, size_t size,
                             std::function<void(boost::system::error_code,
                                 size_t)> callback) override;

    virtual void asyncWrite(const std::vector<boost::asio::const_buffer>& buffers,
                            std::function<void(boost::system::error_code,
                                size_t)> callback) override;

    virtual tcp::socket& socket() override
    {
      return m_spSocket;
    }

    virtual void disconnect() override;


  private:
    tcp::socket m_spSocket;
  };

}

#endif //TCP_CONNECTION_H__


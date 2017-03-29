#ifndef TCP_CONNECTION_H__
#define TCP_CONNECTION_H__

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/regex.hpp>

using boost::asio::ip::tcp;

class InboundConnection {
public:

  virtual ~InboundConnection() = default;

  virtual void asyncRead( void* data, size_t size,
                          boost::function2<void, boost::system::error_code, size_t> callback) = 0;

  virtual void asyncWrite( const void* data, size_t size,
                           boost::function2<void, boost::system::error_code,
                           size_t> callback) = 0;
  virtual void asyncReadUntil(boost::asio::streambuf& buf, const boost::regex& delim,
                              boost::function2<void, boost::system::error_code, size_t> callback) = 0;

  virtual void asyncReadSome(void* data, size_t size,
                             boost::function2<void, boost::system::error_code, size_t> callback) = 0;

  virtual void disconnect() = 0;

  virtual tcp::socket& socket() = 0;

  virtual std::string ip();
};

class TcpInboundConnection : public InboundConnection {

public:
  virtual ~TcpInboundConnection() = default;

  TcpInboundConnection(boost::asio::io_service& io_service);
  virtual void asyncRead( void* data, size_t size,
                          boost::function2<void, boost::system::error_code, size_t> callback) override;

  virtual void asyncReadUntil(boost::asio::streambuf& buf, const boost::regex& delim,
                              boost::function2<void, boost::system::error_code, size_t> callback) override;

  virtual void asyncReadSome(void* data, size_t size,
                             boost::function2<void, boost::system::error_code, size_t> callback) override;

  virtual void asyncWrite( const void* data, size_t size,
                           boost::function2<void, boost::system::error_code,
                           size_t> callback) override;

  virtual tcp::socket& socket() override
  {
    return m_spSocket;
  }

  virtual void disconnect() override;


private:
  tcp::socket m_spSocket;
};

#endif //TCP_CONNECTION_H__

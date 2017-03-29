#ifndef CLIENT_CONNECTION_H__
#define CLIENT_CONNECTION_H__

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

using boost::asio::ip::tcp;

class ClientConnection {
public:

  virtual ~ClientConnection() {}

  ClientConnection(boost::asio::io_service& io_service, const std::string& host,
                   const std::string& port);

  void asyncResolve(
    boost::function2<void, boost::system::error_code, tcp::resolver::iterator>
    callback);

  void setEndpoint(tcp::resolver::iterator iter);

  virtual void asyncConnect(boost::function1<void, boost::system::error_code>
                            callback) = 0;

  virtual void asyncRead( void* data, size_t size,
                          boost::function2<void, boost::system::error_code, size_t> callback) = 0;

  virtual void asyncWrite( const void* data, size_t size,
                           boost::function2<void, boost::system::error_code,
                           size_t> callback) = 0;

  virtual boost::system::error_code write(const void* data, size_t size) = 0;

  virtual tcp::socket& socket() = 0;

  virtual void disconnect()  = 0;

protected:
  tcp::resolver resolver_;
  tcp::resolver::iterator endpointIterator_;
  std::string host_;
  std::string port_;
  boost::asio::io_service& io_service_;
};

class ClientTcpConnection : public ClientConnection {
public:

  virtual ~ClientTcpConnection() = default;

  ClientTcpConnection(boost::asio::io_service& io_service,
                      const std::string& host, const std::string& port);

  virtual void asyncConnect(boost::function1<void, boost::system::error_code>
                            callback) override;

  virtual void asyncRead( void* data, size_t size,
                          boost::function2<void, boost::system::error_code, size_t> callback) override;

  virtual boost::system::error_code write(const void* data, size_t size) override;

  virtual void asyncWrite( const void* data, size_t size,
                           boost::function2<void, boost::system::error_code,
                           size_t> callback) override;

  virtual tcp::socket& socket() override
  {
    return *m_spSocket;
  }

  virtual void disconnect() override;

private:
  typedef std::shared_ptr<tcp::socket> SocketPtr;
  SocketPtr m_spSocket;
};

#endif

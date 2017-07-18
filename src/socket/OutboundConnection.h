#ifndef OUTBOUND_CONNECTION_H__
#define OUTBOUND_CONNECTION_H__

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class OutboundConnection {
public:

  virtual ~OutboundConnection() {}

  OutboundConnection(boost::asio::io_service& io_service, const std::string& host,
                     const std::string& port);

  void asyncResolve(
    std::function<void(boost::system::error_code, tcp::resolver::iterator)>
    callback);

  void setEndpoint(tcp::resolver::iterator iter);

  virtual void asyncConnect(std::function<void(boost::system::error_code)>
                            callback) = 0;

  virtual void asyncRead( void* data, size_t size,
                          std::function<void(boost::system::error_code, size_t)> callback) = 0;

  virtual void asyncWrite( const void* data, size_t size,
                           std::function<void(boost::system::error_code,
                               size_t)> callback) = 0;

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

class OutboundTcpConnection : public OutboundConnection {
public:

  virtual ~OutboundTcpConnection() = default;

  OutboundTcpConnection(boost::asio::io_service& io_service,
                        const std::string& host, const std::string& port);

  virtual void asyncConnect(std::function<void(boost::system::error_code)>
                            callback) override;

  virtual void asyncRead( void* data, size_t size,
                          std::function<void(boost::system::error_code, size_t)> callback) override;

  virtual boost::system::error_code write(const void* data, size_t size) override;

  virtual void asyncWrite( const void* data, size_t size,
                           std::function<void(boost::system::error_code,
                               size_t)> callback) override;

  virtual tcp::socket& socket() override
  {
    return *m_spSocket;
  }

  virtual void disconnect() override;

private:
  typedef std::shared_ptr<tcp::socket> SocketPtr;
  SocketPtr m_spSocket;
};

#endif // OUTBOUND_CONNECTION_H__

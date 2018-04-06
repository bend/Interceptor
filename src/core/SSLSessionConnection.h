#ifndef SSL_SESSION_CONNECTION_H__
#define SSL_SESSION_CONNECTION_H__

#include "SessionConnection.h"

#include <boost/asio/ssl.hpp>

namespace Interceptor {

  class SSLSessionConnection : public SessionConnection {
  public:
    SSLSessionConnection(ParamsPtr params, boost::asio::io_service& ioService);

    virtual ~SSLSessionConnection() = default;
    virtual void init(std::function<void()> callback) override;
    virtual void initConnection() override;

  private:
    void handleSSLHandShake(const boost::system::error_code& error,
                            std::function<void()>& callback);

  private:
    boost::asio::ssl::context m_context;

  };

}

#endif // SSL_SESSION_CONNECTION_H__

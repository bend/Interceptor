#include "SSLSessionConnection.h"

#include "socket/SSLInboundConnection.h"

#include "common/Params.h"
#include "utils/Logger.h"

namespace Interceptor {

  SSLSessionConnection::SSLSessionConnection(ParamsPtr params,
      boost::asio::io_service& ioService)
    : SessionConnection(params, ioService),
      m_context(ioService, boost::asio::ssl::context::sslv23)
  {
    m_context.set_options(
      boost::asio::ssl::context::default_workarounds |
      boost::asio::ssl::context::no_sslv2 |
      boost::asio::ssl::context::single_dh_use);
    m_context.set_verify_mode(boost::asio::ssl::context::verify_none);

    m_context.use_certificate_chain_file(params->m_config->m_pemfile);
    m_context.use_private_key_file(params->m_config->m_pemfile,
                                   boost::asio::ssl::context::pem);
    m_context.use_tmp_dh_file(params->m_config->m_dhfile);
  }

  void SSLSessionConnection::initConnection()
  {
    LOG_DEBUG("SSLSessionConnection::initConnection()");
    m_connection = std::make_shared<Network::SSLInboundConnection>(m_ioService, m_context);
  }

  void SSLSessionConnection::init(std::function<void()> callback)
  {
    LOG_DEBUG("SSLSessionConnection::init()");
    std::shared_ptr<SSLSessionConnection> thisPtr =
      std::dynamic_pointer_cast<SSLSessionConnection>(shared_from_this());

    std::dynamic_pointer_cast<Network::SSLInboundConnection>
    (m_connection)->asyncHandshake(
      std::bind(
        &SSLSessionConnection::handleSSLHandShake,
        thisPtr,
        std::placeholders::_1, callback));
  }

  void SSLSessionConnection::handleSSLHandShake(const boost::system::error_code&
      error, std::function<void()>& callback)
  {
    LOG_DEBUG("SSLSessionConnection::handleSSLHandShake()");

    if (error) {
      LOG_ERROR("Handshake failed with " << m_connection->ip() << " : " <<
                error.message());
    } else {
      LOG_DEBUG("Handshake succeeded with " << m_connection->ip());
      callback();
    }
  }


}

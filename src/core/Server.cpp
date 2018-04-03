#include "Server.h"

#include "Session.h"
#include "socket/InboundConnection.h"
#include "utils/Logger.h"
#include "utils/ServerInfo.h"
#include "common/Params.h"
#include "SessionConnection.h"

using boost::asio::ip::tcp;

namespace Interceptor {

  Server::Server(ParamsPtr params,
                 boost::asio::io_service& ioService)
    : m_params(params),
      m_ioService(ioService),
      m_acceptor(m_ioService, tcp::endpoint(boost::asio::ip::address::from_string(
          m_params->config()->m_listenHost), m_params->config()->m_listenPort))
  {
    LOG_INFO("Launching " << ServerInfo::name() << "/" <<
             ServerInfo::version() <<
             " on " << m_params->config()->m_listenHost << ":" <<
             m_params->config()->m_listenPort);
  }

  Server::~Server()
  {
    LOG_DEBUG("Server::~Server()");

    if (m_acceptor.is_open()) {
      boost::system::error_code ec;
      m_acceptor.cancel(ec);
      m_acceptor.close(ec);
    }
  }

  void Server::init()
  {
    LOG_DEBUG("Server::init()");
    listen();
  }

  void Server::listen()
  {
    LOG_DEBUG("Server::listen()");
    SessionPtr newSession =
      std::make_shared<Session>
      (m_params, m_ioService);
    m_acceptor.async_accept(newSession->m_connection->socket(),
                            std::bind(
                              &Server::handleAccept,
                              shared_from_this(),
                              newSession,
                              std::placeholders::_1)
                           );
  }

  void Server::handleAccept(SessionPtr session,
                            const boost::system::error_code& error)
  {
    LOG_INFO("Server::handleAccept()");

    if ( !error ) {
      session->start();
      LOG_INFO("Incomming connection from " << session->ip());
    } else {
      LOG_INFO("Could not accept connection " << error.message());
    }

    listen();
  }
}

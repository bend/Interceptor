#include "Interceptor.h"

#include "InterceptorSession.h"
#include "Logger.h"
#include "Server.h"

#include <boost/bind.hpp>

using boost::asio::ip::tcp;

Interceptor::Interceptor(const Config::ServerConfig* config, boost::asio::io_service& ioService)
  : m_config(config),
    m_ioService(ioService),
    m_acceptor(m_ioService, tcp::endpoint(boost::asio::ip::address::from_string(m_config->m_listenHost), m_config->m_listenPort))
{
  trace("info") << "Launching " << Server::getName() << "/" << Server::getVersion() << " on " << m_config->m_listenHost << ":" << m_config->m_listenPort;
}

Interceptor::~Interceptor()
{
}

void Interceptor::init()
{
  listen();
}

void Interceptor::listen()
{
  InterceptorSessionPtr newSession = std::make_shared<InterceptorSession>(m_config, m_ioService);
  m_acceptor.async_accept(newSession->socket(),
                          boost::bind(&Interceptor::handleAccept, shared_from_this(), newSession, boost::asio::placeholders::error)
                         );
}

void Interceptor::handleAccept(InterceptorSessionPtr session, const boost::system::error_code& error)
{
  if ( !error ) {
    session->start();
    trace("info") << "Incomming connection from " << session->connection()->ip();
  } else {
    trace("error") << "Could not accept connection " << error.message();
  }
  listen();
}

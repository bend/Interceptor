#include "Interceptor.h"

#include "InterceptorSession.h"
#include "socket/InboundConnection.h"
#include "utils/Logger.h"
#include "utils/Server.h"
#include "cache/generic_cache.h"
#include "common/Params.h"

using boost::asio::ip::tcp;

Interceptor::Interceptor(Params* params,
                         boost::asio::io_service& ioService)
  : m_params(params),
    m_ioService(ioService),
    m_acceptor(m_ioService, tcp::endpoint(boost::asio::ip::address::from_string(
        m_params->config()->m_listenHost), m_params->config()->m_listenPort))
{
  LOG_INFO("Launching " << Http::Server::getName() << "/" <<
           Http::Server::getVersion() <<
           " on " << m_params->config()->m_listenHost << ":" <<
           m_params->config()->m_listenPort);
}

Interceptor::~Interceptor()
{
  delete m_params;
}

void Interceptor::init()
{
  listen();
}

void Interceptor::listen()
{
  InterceptorSessionPtr newSession =
    std::make_shared<InterceptorSession>
    (m_params, m_ioService);
  m_acceptor.async_accept(newSession->socket(),
                          std::bind(&Interceptor::handleAccept, shared_from_this(), newSession,
                                    std::placeholders::_1)
                         );
}

void Interceptor::handleAccept(InterceptorSessionPtr session,
                               const boost::system::error_code& error)
{
  if ( !error ) {
    session->start();
    LOG_INFO("Incomming connection from " << (session->connection() ?
             session->connection()->ip() : " ! already closed ! "));
  } else {
    LOG_INFO("Could not accept connection " << error.message());
  }

  listen();
}

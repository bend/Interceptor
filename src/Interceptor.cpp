#include "Interceptor.h"

#include "InterceptorSession.h"
#include "Logger.h"

#include <boost/bind.hpp>

using boost::asio::ip::tcp;

Interceptor::Interceptor(short port)
  : m_acceptor(m_ioService, tcp::endpoint(tcp::v4(), port))
{
}

Interceptor::~Interceptor()
{
}

void Interceptor::init()
{
  listen();
  boost::asio::io_service::work work(m_ioService);
  m_ioService.run();
}

void Interceptor::listen()
{
  InterceptorSessionPtr newSession = std::make_shared<InterceptorSession>(m_ioService);
  m_acceptor.async_accept(newSession->socket(),
                          boost::bind(&Interceptor::handleAccept, this, newSession, boost::asio::placeholders::error)
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

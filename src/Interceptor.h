#ifndef INTERCEPTOR_H__
#define INTERCEPTOR_H__

#include "InboundConnection.h"

#include "Defs.h"

#include <boost/asio.hpp>


class Interceptor {

  public:
	Interceptor(short port);
	~Interceptor();
	void init();

  private:
	void listen();
	void handleAccept(InterceptorSessionPtr session, const boost::system::error_code& error);

  private:
	boost::asio::io_service m_ioService;
	boost::asio::ip::tcp::acceptor m_acceptor;

};

#endif //INTERCEPTOR_H__

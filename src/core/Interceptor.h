#ifndef INTERCEPTOR_H__
#define INTERCEPTOR_H__

#include "common/Defs.h"

#include <boost/asio.hpp>
#include "Config.h"

class Params;

class Interceptor : public std::enable_shared_from_this<Interceptor> {

public:
  Interceptor(Params* params,
              boost::asio::io_service& ioService);
  ~Interceptor();
  void init();

private:
  void listen();
  void handleAccept(InterceptorSessionPtr session,
                    const boost::system::error_code& error);

private:
  Params* m_params;
  boost::asio::io_service& m_ioService;
  boost::asio::ip::tcp::acceptor m_acceptor;

};

#endif //INTERCEPTOR_H__

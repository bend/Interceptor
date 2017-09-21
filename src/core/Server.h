#ifndef INTERCEPTOR_H__
#define INTERCEPTOR_H__

#include "common/Defs.h"

#include "Config.h"

#include <boost/asio.hpp>


namespace Interceptor {

  class Server : public std::enable_shared_from_this<Server> {

  public:
    Server(ParamsPtr params,
           boost::asio::io_service& ioService);
    ~Server();
    void init();

  private:
    void listen();
    void handleAccept(SessionPtr session,
                      const boost::system::error_code& error);
    void addSession(SessionPtr session);
    void handleSessionRemoved(SessionPtr session);

  private:
    ParamsPtr m_params;
    boost::asio::io_service& m_ioService;
    boost::asio::ip::tcp::acceptor m_acceptor;
  };
}

#endif //INTERCEPTOR_H__

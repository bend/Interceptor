#ifndef INTERCEPTOR_H__
#define INTERCEPTOR_H__

#include "Defs.h"

#include <boost/asio.hpp>
#include "Config.h"

class AbstractCacheHandler;

class Interceptor : public std::enable_shared_from_this<Interceptor> {

public:
  Interceptor(const Config::ServerConfig* config,
              AbstractCacheHandler* cache,
              boost::asio::io_service& ioService);
  ~Interceptor();
  void init();

private:
  void listen();
  void handleAccept(InterceptorSessionPtr session,
                    const boost::system::error_code& error);

private:
  const Config::ServerConfig* m_config;
  AbstractCacheHandler* m_cache;
  boost::asio::io_service& m_ioService;
  boost::asio::ip::tcp::acceptor m_acceptor;

};

#endif //INTERCEPTOR_H__

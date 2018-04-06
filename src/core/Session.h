#ifndef SESSION_H__
#define SESSION_H__

#include "Config.h"
#include "defs.h"

#include "common/defs.h"
#include <boost/asio/io_service.hpp>

namespace Interceptor {

  class SessionTypeDetector;

  class Session : public
    std::enable_shared_from_this<Session>  {

  public:

    Session(ParamsPtr params,
            boost::asio::io_service& ioService);

    ~Session();

    const std::string ip();

    ParamsPtr params() const;

    void start();

  private:
    void doStart();

  private:
    typedef std::shared_ptr<SessionTypeDetector> SessionTypeDetectorPtr;
    ParamsPtr m_params;
    boost::asio::io_service& m_ioService;
    SessionConnectionPtr m_connection;
    SessionTypeDetectorPtr m_detector;
    char m_requestBuffer[4096];

    friend class Server;
  };
}

#endif // SESSION_H__

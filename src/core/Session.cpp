#include "Session.h"
#include "SessionTypeDetector.h"

#include "utils/Logger.h"
#include "SessionConnection.h"

#include "common/Params.h"

namespace Interceptor {

  Session::Session(ParamsPtr params,
                   boost::asio::io_service& ioService)
    : m_params(params),
      m_ioService(ioService)
  {
    m_connection = std::make_shared<SessionConnection>(m_params, m_ioService);
  }

  Session::~Session()
  {
    LOG_DEBUG("Session::~Session()");
  }

  const std::string Session::ip()
  {
    return m_connection->ip();
  }

  ParamsPtr Session::params() const
  {
    return m_params;
  }

  void Session::start()
  {
    LOG_DEBUG("Session::start()");
    m_detector = std::make_shared<SessionTypeDetector>(m_connection);
    m_detector->detectSessionTypeAndHandOver();
  }

}

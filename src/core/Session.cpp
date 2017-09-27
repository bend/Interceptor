#include "Session.h"
#include "SessionTypeDetector.h"

#include "utils/Logger.h"
#include "SessionConnection.h"

#ifdef ENABLE_SSL
#include "SSLSessionConnection.h"
#endif // ENABLE_SSL

#include "common/Params.h"
#include "vars.h"

namespace Interceptor {

  Session::Session(ParamsPtr params,
                   boost::asio::io_service& ioService)
    : m_params(params),
      m_ioService(ioService)
  {
#ifdef ENABLE_SSL

    if (m_params->m_config->m_useSSL) {
      m_connection = std::make_shared<SSLSessionConnection>(m_params, m_ioService);
    } else
#endif // ENABLE_SSL
      m_connection = std::make_shared<SessionConnection>(m_params, m_ioService);

    m_connection->initConnection();
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
    m_connection->init(std::bind(&Session::doStart, shared_from_this()));
  }

  void Session::doStart()
  {
    m_detector = std::make_shared<SessionTypeDetector>(m_connection);
    m_detector->detectSessionTypeAndHandOver();
  }

}

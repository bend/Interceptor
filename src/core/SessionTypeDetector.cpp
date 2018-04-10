#include "SessionTypeDetector.h"

#include "SessionConnection.cpp"
#include "http/Http11SessionHandler.h"
#include "common/AbstractSessionHandler.h"
#ifdef ENABLE_HTTP2
#include "http/http2/Http2SessionHandler.h"
#include "http/http2/Http2SessionUpgrader.h"
#endif // ENABLE_HTTP2

namespace Interceptor {

  SessionTypeDetector::SessionTypeDetector(SessionConnectionPtr connection)
    : m_connection(connection)
  {
    std::memset(m_buffer, 0, sizeof(m_buffer));
  }

  SessionTypeDetector::~SessionTypeDetector()
  {
    LOG_DEBUG("SessionTypeDetector::~SessionTypeDetector()");
  }

  void SessionTypeDetector::detectSessionTypeAndHandOver()
  {
    LOG_DEBUG("SessionTypeDetector::detectSessionTypeAndHandOver()");
    readFirstPacketToIdentify();
  }

  void SessionTypeDetector::readFirstPacketToIdentify()
  {
    LOG_DEBUG("SessionTypeDetector::readFirstPacketToIdentify()");
    m_connection->asyncReadSome(m_buffer, sizeof(m_buffer),
                                std::bind(&SessionTypeDetector::handleFirstPacketRead,
                                          shared_from_this(), std::placeholders::_1, std::placeholders::_2));
  }

  void SessionTypeDetector::handleFirstPacketRead(const boost::system::error_code&
      error, size_t bytesTransferred)
  {
    LOG_DEBUG("SessionTypeDetector::handleFirstPacketRead()");

    if ( error ) {
      LOG_ERROR("SessionTypeDetector::handleFirstPacketRead() : Error while reading");
      m_connection->closeConnection();
      return;
    }

    switch (detectSessionType(m_buffer, bytesTransferred)) {
      case HTTP11:
        handleHttp11Session(m_buffer, bytesTransferred);
        break;

#ifdef ENABLE_HTTP2
      case HTTP2:
        LOG_INFO("HTTP2 requested, NYI - serving HTTP/1.1");
        handleHttp2Session(m_buffer, bytesTransferred);
        break;

      case HTTP2Upgrade:
        LOG_INFO("HTTP2 Upgrade requested, NYI - serving HTTP/1.1");
        handleHttp2UpgradeSession(m_buffer, bytesTransferred);
        break;
#endif // ENABLE_HTTP2

      default:
        LOG_ERROR("SessionTypeDetector::handleFirstPacketRead() - Unknown packet type : "
                  << bytesTransferred << " [" << std::string(m_buffer, bytesTransferred) << "]");
        m_connection->closeConnection();
    }
  }

  void SessionTypeDetector::handleHttp11Session(const char* data, size_t len)
  {
    m_sessionHandler = std::make_shared<Http::Http11SessionHandler>(m_connection);
    m_sessionHandler->transferSession(data, len);
  }

#ifdef ENABLE_HTTP2
  void SessionTypeDetector::handleHttp2Session(const char* data, size_t len)
  {
    m_sessionHandler = std::make_shared<Http::Http2SessionHandler>(m_connection);
    m_sessionHandler->transferSession(data, len);
  }


  void SessionTypeDetector::handleHttp2UpgradeSession(const char* data,
      size_t len)
  {
    m_sessionHandler = std::make_shared<Http::Http2SessionUpgrader>(m_connection);
    m_sessionHandler->transferSession(data, len);
  }
#endif // ENABLE_HTTP2

  SessionTypeDetector::SessionType SessionTypeDetector::detectSessionType(
    const char* data, size_t len)
  {
    LOG_DEBUG("SessionTypeDetector::detectSessionType()");
    std::string str(data, len);

    if ( str.find("h2c") != std::string::npos
         && str.find("Upgrade") != std::string::npos) {
      return HTTP2Upgrade;
    } else if (str.find("HTTP/1.1") != std::string::npos) {
      return HTTP11;
    } else {
      return Other;
    }
  }


};

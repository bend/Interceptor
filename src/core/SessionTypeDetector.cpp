#include "SessionTypeDetector.h"

#include "SessionConnection.cpp"
#include "http/HTTP1XSessionHandler.h"
#include "common/AbstractSessionHandler.h"

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
      case HTTP1X:
        m_sessionHandler = std::make_shared<Http::HTTP1XSessionHandler>(m_connection);
        m_sessionHandler->transferSession(m_buffer, bytesTransferred);
        break;

      default:
        LOG_ERROR("SessionTypeDetector::handleFirstPacketRead() - Unknown packet type");
        m_connection->closeConnection();
    }
  }

  SessionTypeDetector::SessionType SessionTypeDetector::detectSessionType(
    const char* data, size_t len)
  {
    LOG_DEBUG("SessionTypeDetector::detectSessionType()");
    std::string str(data, len);

    if (str.find("HTTP/1") != std::string::npos) {
      return HTTP1X;
    } else {
      return Other;
    }
  }

};

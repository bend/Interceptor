#include "SessionTypeDetector.h"

#include "SessionConnection.cpp"
#include "http/HTTP11SessionHandler.h"
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
      case HTTP11:
        handleHTTP11Session(m_buffer, bytesTransferred);
        break;

      default:
        LOG_ERROR("SessionTypeDetector::handleFirstPacketRead() - Unknown packet type : "
                  << bytesTransferred << " [" << std::string(m_buffer, bytesTransferred) << "]");
        m_connection->closeConnection();
    }
  }

  void SessionTypeDetector::handleHTTP11Session(const char* data, size_t len)
  {
    m_sessionHandler = std::make_shared<Http::HTTP11SessionHandler>(m_connection);
    m_sessionHandler->transferSession(data, len);
  }

  SessionTypeDetector::SessionType SessionTypeDetector::detectSessionType(
    const char* data, size_t len)
  {
    LOG_DEBUG("SessionTypeDetector::detectSessionType()");
    std::string str(data, len);

    if (str.find("HTTP/1.1") != std::string::npos) {
      return HTTP11;
    } else {
      return Other;
    }
  }

};

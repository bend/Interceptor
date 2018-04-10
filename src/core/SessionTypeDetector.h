#ifndef SESSION_TYPE_DETECTOR_H__
#define SESSION_TYPE_DETECTOR_H__

#include "defs.h"
#include <boost/system/error_code.hpp>

namespace Interceptor {

  class AbstractSessionHandler;

  class SessionTypeDetector : public
    std::enable_shared_from_this<SessionTypeDetector> {
  public:
    SessionTypeDetector(SessionConnectionPtr connection);
    ~SessionTypeDetector();
    void detectSessionTypeAndHandOver();

  private:
    enum SessionType {
      HTTP11,
      HTTP2,
      HTTP2Upgrade,
      Other
    };

  private:
    void readFirstPacketToIdentify();
    void handleFirstPacketRead(const boost::system::error_code& error,
                               size_t bytesTransferred);
    void handleHTTP11Session(const char* data, size_t len);
    void handleHTTP2Session(const char* data, size_t len);
    void handleHTTP2UpgradeSession(const char* data, size_t len);
    static SessionType detectSessionType(const char* data, size_t len);

  private:
    typedef std::shared_ptr<AbstractSessionHandler> AbstractSessionHandlerPtr;
    SessionConnectionPtr m_connection;
    AbstractSessionHandlerPtr m_sessionHandler;
    char m_buffer[4096];
  };


};

#endif // SESSION_TYPE_DETECTOR_H__

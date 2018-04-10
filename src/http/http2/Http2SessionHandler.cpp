#include "Http2SessionHandler.h"

#include "Request.h"
#include "Reply.h"
#include "HttpException.h"

#include "core/SessionConnection.h"
#include "utils/Logger.h"
#include "common/Buffer.h"


namespace Interceptor::Http {

  Http2SessionHandler::Http2SessionHandler(SessionConnectionPtr connection) :
    AbstractSessionHandler(connection)
  {
  }

  Http2SessionHandler::~Http2SessionHandler()
  {
    LOG_DEBUG("Http2SessionHandler::~Http2SessionHandler()");
  }

  void Http2SessionHandler::transferSession(const char* data, size_t bytes)
  {
    LOG_DEBUG("Http2SessionHandler::transferSession()");

    if (bytes > 0) {
      processData(data, bytes);
    }

    read();
  }

  void Http2SessionHandler::read()
  {
    m_connection->asyncReadSome(m_requestBuffer, sizeof(m_requestBuffer),
                                std::bind(&Http2SessionHandler::handleHttpRequestRead, shared_from_this(),
                                          std::placeholders::_1, std::placeholders::_2)
                               );
  }

  void Http2SessionHandler::handleHttpRequestRead(const boost::system::error_code&
      error, size_t bytesTransferred)
  {
    LOG_DEBUG("Http2SessionHandler::handleHttpRequestRead()");

    if (!error) {
      processData(m_requestBuffer, bytesTransferred);
    } else {
      if (error != boost::asio::error::eof
          && error != boost::asio::error::connection_reset) {
        if (m_connection && m_connection->ip().length() > 0) {
          LOG_ERROR("Error reading request from " << m_connection->ip());
        }
      }

      m_connection->closeConnection();
    }
  }

  void Http2SessionHandler::processData(const char* data, size_t length)
  {
    LOG_INFO("Request read from " << m_connection->ip());

    try {

      // Previous request already done, this is a new request
      if (!m_request || m_request->completed() ) {
        m_request = std::make_shared<Http::Request>(m_connection);
      }

      // Append data to current request
      m_request->appendData(data, length);


    } catch (HttpException& e) {
      if (!m_reply) {
        m_reply = std::make_shared<Http::Reply>(m_request);
      }

      m_reply->declineRequest(e.code());
    }
  }

}

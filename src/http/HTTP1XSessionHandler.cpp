#include "HTTP1XSessionHandler.h"

#include "core/SessionConnection.h"
#include "utils/Logger.h"
#include "Request.h"
#include "Reply.h"

namespace Interceptor::Http {

  HTTP1XSessionHandler::HTTP1XSessionHandler(SessionConnectionPtr connection) :
    AbstractSessionHandler(connection)
  {
  }

  HTTP1XSessionHandler::~HTTP1XSessionHandler()
  {
    LOG_DEBUG("HTTP1XSessionHandler::~HTTP1XSessionHandler()");
  }

  void HTTP1XSessionHandler::transferSession(const char* data, size_t bytes)
  {
    LOG_DEBUG("HTTP1XSessionHandler::transferSession()");
    processData(data, bytes);
    read();
  }

  void HTTP1XSessionHandler::read()
  {
    m_connection->asyncReadSome(m_requestBuffer, sizeof(m_requestBuffer),
                                std::bind(&HTTP1XSessionHandler::handleHttpRequestRead, shared_from_this(),
                                          std::placeholders::_1, std::placeholders::_2)
                               );
  }

  void HTTP1XSessionHandler::handleHttpRequestRead(const boost::system::error_code&
      error, size_t bytesTransferred)
  {
    LOG_DEBUG("HTTP1XSessinHandlerhandleHttpRequestRead()");

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

  void HTTP1XSessionHandler::processData(const char* data, size_t length)
  {
    LOG_INFO("Request read from " << m_connection->ip());

    // Previous request already done, this is a new request
    if (!m_request || m_request->completed() ) {
      m_request = std::make_shared<Http::Request>(m_connection);
    }

    // Append data to current request
    Http::Code ret =  m_request->appendData(data, length);

    if (ret != Http::Code::Ok) {
      if (!m_reply) {
        m_reply = std::make_shared<Http::Reply>(m_request);
      }

      m_reply->declineRequest(ret);
      return;
    }

    if (!m_request->headersReceived()) {
      read();
    } else  {
      // Complete headers received
      m_reply = std::make_shared<Http::Reply>(m_request);
      m_reply->process();
      read();
    }

  }


}

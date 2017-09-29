#include "HTTP11SessionHandler.h"

#include "Request.h"
#include "Reply.h"
#include "HttpException.h"

#include "core/SessionConnection.h"
#include "utils/Logger.h"
#include "common/Buffer.h"


namespace Interceptor::Http {

  HTTP11SessionHandler::HTTP11SessionHandler(SessionConnectionPtr connection) :
    AbstractSessionHandler(connection)
  {
  }

  HTTP11SessionHandler::~HTTP11SessionHandler()
  {
    LOG_DEBUG("HTTP11SessionHandler::~HTTP11SessionHandler()");
  }

  void HTTP11SessionHandler::transferSession(const char* data, size_t bytes)
  {
    LOG_DEBUG("HTTP11SessionHandler::transferSession()");
    processData(data, bytes);
    read();
  }

  void HTTP11SessionHandler::read()
  {
    m_connection->asyncReadSome(m_requestBuffer, sizeof(m_requestBuffer),
                                std::bind(&HTTP11SessionHandler::handleHttpRequestRead, shared_from_this(),
                                          std::placeholders::_1, std::placeholders::_2)
                               );
  }

  void HTTP11SessionHandler::handleHttpRequestRead(const boost::system::error_code&
      error, size_t bytesTransferred)
  {
    LOG_DEBUG("HTTP11SessinHandlerhandleHttpRequestRead()");

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

  void HTTP11SessionHandler::processData(const char* data, size_t length)
  {
    LOG_INFO("Request read from " << m_connection->ip());

    try {

      // Previous request already done, this is a new request
      if (!m_request || m_request->completed() ) {
        m_request = std::make_shared<Http::Request>(m_connection);
      }

      // Append data to current request
      m_request->appendData(data, length);

      send101Continue();

      if (!m_request->headersReceived()) {
        read();
      } else  {
        // Complete headers received
        m_reply = std::make_shared<Http::Reply>(m_request);
        m_reply->process();
        read();
      }

    } catch (HttpException& e) {
      if (!m_reply) {
        m_reply = std::make_shared<Http::Reply>(m_request);
      }

      m_reply->declineRequest(e.code());
    }
  }

  void HTTP11SessionHandler::send101Continue()
  {
    BufferPtr buf = std::make_shared<Buffer>();
    buf->m_buffers.push_back(buf->buf("HTTP/1.1 100 Continue\r\n\r\n"));
    m_connection->postReply(buf);
  }

}

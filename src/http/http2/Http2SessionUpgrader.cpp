#include "Http2SessionUpgrader.h"

#include "Request.h"
#include "Reply.h"
#include "Http2UpgradeReply.h"
#include "HttpException.h"
#include "Http2SessionHandler.h"
#include "core/SessionConnection.h"
#include "utils/Logger.h"
#include "common/Buffer.h"


namespace Interceptor::Http {

  Http2SessionUpgrader::Http2SessionUpgrader(SessionConnectionPtr connection) :
    AbstractSessionHandler(connection)
  {
  }

  Http2SessionUpgrader::~Http2SessionUpgrader()
  {
    LOG_DEBUG("Http2SessionUpgrader::~Http2SessionUpgrader()");
  }

  void Http2SessionUpgrader::transferSession(const char* data, size_t bytes)
  {
    LOG_DEBUG("Http2SessionUpgrader::transferSession()");
    processData(data, bytes);
    read();
  }

  void Http2SessionUpgrader::read()
  {
    m_connection->asyncReadSome(m_requestBuffer, sizeof(m_requestBuffer),
                                std::bind(&Http2SessionUpgrader::handleHttpRequestRead, shared_from_this(),
                                          std::placeholders::_1, std::placeholders::_2)
                               );
  }

  void Http2SessionUpgrader::handleHttpRequestRead(const boost::system::error_code&
      error, size_t bytesTransferred)
  {
    LOG_DEBUG("Http2SessionUpgraderhandleHttpRequestRead()");

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

  void Http2SessionUpgrader::processData(const char* data, size_t length)
  {
    LOG_INFO("Request read from " << m_connection->ip());

    try {

      // Previous request already done, this is a new request
      if (!m_request || m_request->completed() ) {
        m_request = std::make_shared<Http::Request>(m_connection);
      }

      // Append data to current request
      m_request->appendData(data, length);

      if (!m_request->headersReceived()) {
        read();
      } else  {
        m_request->parse();
        auto reply = std::make_shared<Http::Http2UpgradeReply>(m_request)->buildReply();
        m_request->connection()->postReply(reply);
		std::make_shared<Http2SessionHandler>(m_request->connection())->transferSession(nullptr, 0);
      }

    } catch (HttpException& e) {
      std::make_shared<Http::Reply>(m_request)->declineRequest(e.code());
    }
  }

  void Http2SessionUpgrader::send101Continue()
  {
    BufferPtr buf = std::make_shared<Buffer>();
    buf->m_buffers.push_back(buf->buf("HTTP/1.1 100 Continue\r\n\r\n"));
    m_connection->postReply(buf);
  }

}

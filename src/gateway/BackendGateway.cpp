#include "BackendGateway.h"
#include "utils/Logger.h"

namespace Interceptor {

  BackendGateway::BackendGateway(HttpRequestPtr request)
    : AbstractGateway(request)
  {}

  BackendGateway::~BackendGateway()
  {
    LOG_DEBUG("BackendGateway::~BackendGateway()");
  }

  void BackendGateway::reset()
  {
    LOG_DEBUG("BackendGateway::reset()");
    m_request.reset();
    m_connection->reset();
    m_signalCon.disconnect();
  }

  void BackendGateway::setConnection(AbstractConnectorPtr connection)
  {
    LOG_DEBUG("BackendGateway::setConnection()");
    assert(connection);
    m_connection = std::move(connection);
    m_connection->connect();
  }

  AbstractConnectorPtr BackendGateway::takeConnection()
  {
    LOG_DEBUG("BackendGateway::takeConnection()");
    assert(m_connection);
    return std::move(m_connection);
  }

  void BackendGateway::handleRequest(
    std::function<void(Http::Code, std::stringstream*)> callback)
  {
    LOG_DEBUG("BackendGateway::handleRequest");
    m_callback = callback;
    auto data = m_request->request();
    forward(data);
    m_signalCon = m_request->hasMoreData().connect(std::bind(
                    &BackendGateway::hasMoreData, shared_from_this()));
  }

  void BackendGateway::hasMoreData()
  {
    LOG_DEBUG("BackendGateway::hasMoreData()");
    auto data = m_request->popRequest();
    forward(data);
  }

  void BackendGateway::forward(Packet packet)
  {
    m_connection->forward(packet,
    std::bind([ = ]( Http::Code code, HttpRequestPtr request) {
      LOG_DEBUG("Gateway fw : " << (int) code );

      if (code != Http::Code::Ok) {
        std::stringstream stream;
        m_callback(code, nullptr);
      } else {
        if (request->received() && !request->hasData()) {
          m_connection->readReply(m_callback);
        }
      }

    }, std::placeholders::_1, m_request));
  }

}


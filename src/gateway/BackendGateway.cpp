#include "BackendGateway.h"
#include "utils/Logger.h"

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
  auto data = m_request->request();
  forward(data, callback);
  std::weak_ptr<BackendGateway> wp = shared_from_this();
  m_signalCon = m_request->hasMoreData().connect(
	  std::bind([=]() {
		auto ptr = wp.lock();
		if(ptr)
		  ptr->hasMoreData(callback);
		}));
}

void BackendGateway::hasMoreData(std::function<void(Http::Code, std::stringstream*)> callback) {
  LOG_DEBUG("BackendGateway::hasMoreData()");
  auto data = m_request->popRequest();
  forward(data, callback);
}

void BackendGateway::forward(Packet packet,
                             std::function<void(Http::Code, std::stringstream*)> callback)
{
  m_connection->forward(packet,
	  std::bind([ = ]( Http::Code code, HttpRequestPtr request) {
		LOG_DEBUG("Gateway fw : " << (int) code );
    if (code != Http::Code::Ok) {
      std::stringstream stream;
      callback(code, nullptr);
    } else {
      if (request->completed())
        ;

      //m_connection->
    }
	
}, std::placeholders::_1, m_request));
}




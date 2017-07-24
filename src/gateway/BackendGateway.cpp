#include "BackendGateway.h"
#include "utils/Logger.h"

BackendGateway::BackendGateway(HttpRequestPtr request)
  : AbstractGateway(request)
{}

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
  std::function<void(Http::Code, std::stringstream&)> callback)
{
  LOG_DEBUG("BackendGateway::handleRequest");
  auto data = m_request->request();
  forward(std::get<const char*>(data), std::get<size_t>(data), callback);

  m_request->hasMoreData().connect(std::bind([=]() {
	auto data = m_request->popRequest();
	forward(std::get<const char*>(data), std::get<size_t>(data), callback);
		}));

}

void BackendGateway::forward(const char* data, size_t length, std::function<void(Http::Code, std::stringstream&)> callback) 
{
  m_connection->forward(data, length, 
	  std::bind([=](const Http::Code code) {
		  if(code != Http::Code::Ok){
			std::stringstream stream;
			callback(code, stream);
		  } else {
			if(m_request->completed())
			;
			  //m_connection->
		  }
		}, std::placeholders::_1));
}




#include "HttpReply.h"

#include "HttpRequest.h"
#include "HttpHeaders.h"
#include "InterceptorSession.h"
#include "InboundConnection.h"
#include "Utils.h"
#include "Config.h"


#include <boost/bind.hpp>


HttpReply::HttpReply(HttpRequestPtr request)
  : m_request(request),
    m_replyHeaders(nullptr)
{
}

HttpReply::~HttpReply()
{
  delete m_replyHeaders;
}


void HttpReply::process()
{
  if ( !m_request->headersReceived() )
    // TODO error
    return;

  m_request->parse();
  
  if (m_replyHeaders)
    delete m_replyHeaders;

  m_replyHeaders = new HttpHeaders();
  m_replyHeaders->addGeneralHeaders();


  if(m_request->status() != Http::ErrorCode::Ok) 
  {
	std::stringstream stream;
	m_replyHeaders = new HttpHeaders();
	m_replyHeaders->addGeneralHeaders();
	sendErrorResponse(m_request->status(), stream, true);
	return;
  }

  switch (m_request->method()) {
  case HttpRequest::GET:
    handleGetRequest();
    break;
  case HttpRequest::POST:
    break;
  default:
    break;
  }
}

void HttpReply::handleGetRequest()
{
  if (!m_request->hasMatchingSite()) {
	std::stringstream response;
	sendErrorResponse(Http::NotFound, response, true);
    return;
  }

  const Config::ServerConfig::Site* site = m_request->matchingSite();

  std::stringstream response;
  std::string page;
  size_t pageLength = 0;
  unsigned char* pageData = nullptr;

  if( m_request->index() == "" || m_request->index() == "/") 
  {
	// This request does not contain a filename, we will use a page from try-file
	std::vector<std::string> tryFiles = site->m_tryFiles;
	bool found = false;
	for(const auto& index : tryFiles) 
	{
	  page = site->m_docroot + index;

	  if (!Utils::readFile(page, &pageData, pageLength)) {
		delete[] pageData;
	  } else 
	  {
		found = true;
		break;
	  }
	}
	if(!found)
	{
	  sendErrorResponse(Http::ErrorCode::NotFound, response);
	}
	
  } else 
  {
	// This request contains the filename, hence we should not try a filename from the list of try-files
	page = site->m_docroot + m_request->index();

	if (!Utils::readFile(page, &pageData, pageLength)) {
	  sendErrorResponse(Http::ErrorCode::NotFound, response);
	  return;
	}

  }

  response << m_request->httpVersion() << " " << Http::ErrorCode::Ok << Http::stringValue(Http::ErrorCode::Ok) << "\r\n";
  m_replyHeaders->addHeader("Content-Type", Utils::getMimeType(page));
  m_replyHeaders->addHeader("Content-Length", pageLength);
  m_replyHeaders->serialize(response);

  send(response);

  InterceptorSession::Packet* packet = new InterceptorSession::Packet();
  packet = new InterceptorSession::Packet(pageData, pageLength);
  m_request->session()->postResponse(packet);
  m_request->setCompleted(true);
}

void HttpReply::send(std::stringstream& stream)
{
  InterceptorSession::Packet* packet = new InterceptorSession::Packet();
  size_t size = stream.str().length();
  unsigned char* data = new unsigned char[size];
  memcpy(data, stream.str().data(), size);
  packet->m_data = data;
  packet->m_size = size;
  m_request->session()->postResponse(packet);
}

void HttpReply::sendErrorResponse(Http::ErrorCode error, std::stringstream& response, bool closeConnection)
{
	  response << m_request->httpVersion() 
			   << " " << error << " " 
			   << Http::stringValue(error) << "\r\n";
	  
	  size_t pageLength = 0;
	  unsigned char* pageData = nullptr;
	  std::string resp;
	  bool found = false;

	  const ErrorPageMap& map = m_request->hasMatchingSite() ? 
		m_request->matchingSite()->m_errorPages : m_request->session()->config()->m_errorPages;

		if(map.count(std::to_string(error)) > 0 ) {
			std::string url = map.at(std::to_string(error));

			if (!Utils::readFile(url, &pageData, pageLength)) {
			  delete[] pageData;
			} else 
			  found = true;
		  }
	  
	  if(!found) {
		resp = "<html> <body> " + Http::stringValue(error) +" </body>  </html>";
		pageLength = resp.length();
	  }

	  m_replyHeaders->addHeader("Content-Length", pageLength);

	  if( closeConnection)
		m_replyHeaders->addHeader("Connection", "close");

	  m_replyHeaders->serialize(response);
	  
	  if(!found)
		response << resp;

	  send(response);

	  if(found) {
		InterceptorSession::Packet* packet = new InterceptorSession::Packet();
		packet = new InterceptorSession::Packet(pageData, pageLength);
		m_request->session()->postResponse(packet);
	  }


	  m_request->setCompleted(true);
	  
	  if( closeConnection)
		m_request->session()->closeConnection();
}


#include "Http.h"

namespace Interceptor::Http {

  void serializeHttpCode(StatusCode error, std::stringstream& stream)
  {
    switch (error) {
      case StatusCode::SwitchingProtocol:
        stream << "101 SwitchingProtocol" << "\r\n";
        break;

      case StatusCode::Ok:
        stream << "200 OK" << "\r\n";
        break;

      case StatusCode::PartialContent:
        stream << "206 Partial Content" << "\r\n";
        break;

      case StatusCode::MovedPermanently:
        stream << "301 Moved Permanently" << "\r\n";
        break;

      case StatusCode::Found:
        stream << "302 Found" << "\r\n";
        break;

      case StatusCode::NotModified:
        stream << "304 Not Modified" << "\r\n";
        break;

      case StatusCode::BadRequest:
        stream << "400 Bad Request" << "\r\n";
        break;

      case StatusCode::Unauthorized:
        stream << "401 Unauthorized" << "\r\n";
        break;

      case StatusCode::Forbidden:
        stream << "403 Forbidden" << "\r\n";
        break;

      case StatusCode::NotFound:
        stream  << "404 Not Found" << "\r\n";
        break;

      case StatusCode::RequestEntityTooLarge:
        stream << "413 Request Entity Too Large" << "\r\n";
        break;

      case StatusCode::RequestRangeNotSatisfiable:
        stream << "416 Request Range Not Satisfiable" << "\r\n";
        break;

      case StatusCode::UnprocessableEntity:
        stream << "422 Unprocessable Entity" << "\r\n";
        break;

      case StatusCode::InternalServerError:
        stream << "500 Internal Server Error" << "\r\n";
        break;

      case StatusCode::NotImplemented:
        stream << "501 Not Implemented" << "\r\n";
        break;

      case StatusCode::ServiceUnavailable:
        stream << "503 Service Not Available" << "\r\n";
        break;

      case StatusCode::HttpVersionNotSupported:
        stream << "505 HTTP Version Not Supported" << "\r\n";
        break;

      default:
        stream << (short)error << " Unknown" << "\r\n";
        break;
    }
  }

  StatusCode convertToHttpCode(const boost::system::error_code& error)
  {
    switch (error.value()) {
      case boost::system::errc::success:
        return StatusCode::Ok;

      case boost::system::errc::network_down:
      case boost::system::errc::broken_pipe:
      case boost::system::errc::network_unreachable:
        return StatusCode::ServiceUnavailable;

      default:
        return StatusCode::InternalServerError;
    }
  }


}

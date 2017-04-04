#include "Http.h"

namespace Http {

  void stringValue(Code error, std::stringstream& stream)
  {
    switch (error) {
      case Code::Ok:
        stream << "200 OK" << "\r\n";
        break;

      case Code::PartialContent:
        stream << "206 Partial Content" << "\r\n";
        break;

      case Code::BadRequest:
        stream << "400 Bad Request" << "\r\n";
        break;

      case Code::NotFound:
        stream  << "404 Not Found" << "\r\n";
        break;

      case Code::HttpVersionNotSupported:
        stream << "505 HTTP Version Not Supported" << "\r\n";
        break;

      default:
        stream << (short)error << " Unknown" << "\r\n";
        break;
    }
  }


}
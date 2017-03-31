#ifndef HTTP_H_
#define HTTP_H_

#include <sstream>

namespace Http {
  enum class ErrorCode : short {
    Ok = 200,
    NotFound = 404,
    BadRequest = 400,
    HttpVersionNotSupported = 505
  };

  enum class Method : char {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATCH
  };

  static void stringValue(ErrorCode error, std::stringstream& stream)
  {
    switch (error) {
      case ErrorCode::Ok:
        stream << "200 OK" << "\r\n";
        break;

      case ErrorCode::NotFound:
        stream  << "404 Not Found" << "\r\n";
        break;

      case ErrorCode::BadRequest:
        stream << "404 Bad Request" << "\r\n";
        break;

      case ErrorCode::HttpVersionNotSupported:
        stream << "505 HTTP Version Not Supported" << "\r\n";
        break;

      default:
        stream << (short)error << " Unknown" << "\r\n";
        break;
    }
  }

};

#endif // HTTP_H_

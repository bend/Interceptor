#ifndef HTTP_H_
#define HTTP_H_

#include <sstream>

class Http {
public:
  enum ErrorCode : short {
    Ok = 200,
    NotFound = 404,
    BadRequest = 400,
    HttpVersionNotSupported = 505
  };

  static void stringValue(ErrorCode error, std::stringstream& stream)
  {
    switch (error) {
      case Ok:
        stream << "200 OK" << "\r\n";
        break;

      case NotFound:
        stream  << "404 Not Found" << "\r\n";
        break;

      case BadRequest:
        stream << "404 Bad Request" << "\r\n";
        break;

      case HttpVersionNotSupported:
        stream << "505 HTTP Version Not Supported" << "\r\n";
        break;

      default:
        stream << error << " Unknown" << "\r\n";
        break;
    }
  }

};

#endif // HTTP_H_

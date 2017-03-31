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

  void stringValue(ErrorCode error, std::stringstream& stream);

};

#endif // HTTP_H_

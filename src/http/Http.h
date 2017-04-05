#ifndef HTTP_H_
#define HTTP_H_

#include <sstream>

namespace Http {
  enum class Code : short {
    Ok = 200,
    PartialContent = 206,
    BadRequest = 400,
    NotFound = 404,
    HttpVersionNotSupported = 505
  };

  enum class Method : char {
	ERR,
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

  void stringValue(Code error, std::stringstream& stream);

};

#endif // HTTP_H_

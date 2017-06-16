#ifndef HTTP_H_
#define HTTP_H_

#include <sstream>

namespace Http {
  enum class Code : short {
    Ok								= 200,
    PartialContent					= 206,
    BadRequest						= 400,
    Forbidden						= 403,
    NotFound						= 404,
    RequestRangeNotSatisfiable		= 416,
    UnprocessableEntity				= 422,
    InternalServerError				= 500,
    NotImplemented					= 501,
    HttpVersionNotSupported			= 505
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

}

#endif // HTTP_H_

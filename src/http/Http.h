#ifndef HTTP_H_
#define HTTP_H_

#include <sstream>

#include <boost/system/error_code.hpp>

namespace Interceptor::Http {
  enum class Code : short {
    Ok								= 200,
    PartialContent					= 206,
    NotModified						= 304,
    BadRequest						= 400,
    Forbidden						= 403,
    NotFound						= 404,
    RequestEntityTooLarge			= 413,
    RequestRangeNotSatisfiable		= 416,
    UnprocessableEntity				= 422,
    InternalServerError				= 500,
    NotImplemented					= 501,
    ServiceUnavailable				= 503,
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

  Code convertToHttpCode(const boost::system::error_code& error);

}

#endif // HTTP_H_

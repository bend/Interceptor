set(libhttp_src
  Headers.cpp
  Request.cpp
  Reply.cpp
  CommonReply.cpp
  GetReply.cpp
  HeadReply.cpp
  ErrorReply.cpp
  Http.cpp
  HTTP1XSessionHandler.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_http STATIC ${libhttp_src})

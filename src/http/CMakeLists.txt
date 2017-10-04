set(libhttp_src
  Headers.cpp
  Request.cpp
  Reply.cpp
  CommonReply.cpp
  GetReply.cpp
  PostReply.cpp
  HeadReply.cpp
  ErrorReply.cpp
  RedirectReply.cpp
  Http.cpp
  HTTP11SessionHandler.cpp
  Encoder.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_http STATIC ${libhttp_src})

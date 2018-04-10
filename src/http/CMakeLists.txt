set(libhttp_src
  Headers.cpp
  Request.cpp
  Reply.cpp
  CommonReply.cpp
  GetReply.cpp
  PostReply.cpp
  HeadReply.cpp
  ErrorReply.cpp
  AuthenticationReply.cpp
  RedirectReply.cpp
  Http.cpp
  Http11SessionHandler.cpp
  Encoder.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

if(ENABLE_HTTP2)
	subdirs(http2)
endif(ENABLE_HTTP2)

add_library(interceptor_http STATIC ${libhttp_src})


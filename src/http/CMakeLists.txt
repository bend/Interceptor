set(libhttp_src
  Headers.cpp
  Request.cpp
  Reply.cpp
  Http.cpp
  HTTP1XSessionHandler.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_http STATIC ${libhttp_src})

set(libhttp_src
  HttpHeaders.cpp
  HttpRequest.cpp
  HttpReply.cpp
  Http.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_http STATIC ${libhttp_src})

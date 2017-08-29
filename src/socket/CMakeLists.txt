set(libsocket_src
  InboundConnection.cpp
  OutboundConnection.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_socket STATIC ${libsocket_src})

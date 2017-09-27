set(libsocket_src
  InboundConnection.cpp
  OutboundConnection.cpp
)

if(ENABLE_SSL)
  set(libsocket_src
     ${libsocket_src}
     SSLInboundConnection.cpp
  )
endif(ENABLE_SSL)


INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_socket STATIC ${libsocket_src})

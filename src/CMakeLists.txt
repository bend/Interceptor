set(src
  Interceptor.cpp
  InterceptorSession.cpp
  InboundConnection.cpp
  HttpHeaders.cpp
  HttpRequest.cpp
  HttpReply.cpp
  #  OutboundConnection.cpp
  Logger.cpp
  Server.cpp
  Utils.cpp
  Config.cpp
  main.cpp
)

INCLUDE_DIRECTORIES(".")

add_executable(interceptor ${src})
target_link_libraries(interceptor ${LIBS})

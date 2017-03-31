set(src
  core/Interceptor.cpp
  core/InterceptorSession.cpp
  socket/InboundConnection.cpp
  http/HttpHeaders.cpp
  http/HttpRequest.cpp
  http/HttpReply.cpp
  #  OutboundConnection.cpp
  Logger.cpp
  Server.cpp
  FileUtils.cpp
  Config.cpp
  main.cpp
)

INCLUDE_DIRECTORIES(
  "."
  )

add_executable(interceptor ${src})
target_link_libraries(interceptor ${LIBS})

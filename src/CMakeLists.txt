set(src
  core/Interceptor.cpp
  core/InterceptorSession.cpp
  core/Config.cpp

  socket/InboundConnection.cpp

  http/HttpHeaders.cpp
  http/HttpRequest.cpp
  http/HttpReply.cpp
  http/Http.cpp

  #  OutboundConnection.cpp

  utils/Logger.cpp
  utils/Server.cpp
  utils/FileUtils.cpp

  main.cpp
)

if(ENABLE_LOCAL_CACHE)
  set(src ${src} cache/CacheHandler.cpp)
else(ENABLE_LOCAL_CACHE)
  set(src ${src} cache/BasicCacheHandler.cpp)
endif(ENABLE_LOCAL_CACHE)


INCLUDE_DIRECTORIES(
  "."
  )

add_executable(interceptor ${src})
target_link_libraries(interceptor ${LIBS})
install(TARGETS interceptor DESTINATION bin)

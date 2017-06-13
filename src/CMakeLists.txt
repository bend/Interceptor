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
  
  Main.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

subdirs(cache)

set(LIBS
  ${LIBS}
  interceptor_cache
)

if(ENABLE_LOCAL_CACHE)
  include_directories(
	${FAM_INCLUDE_DIR}
	)
  set(LIBS ${LIBS} ${FAM_LIBRARIES})
endif(ENABLE_LOCAL_CACHE)

add_executable(interceptor ${src})
target_link_libraries(interceptor ${LIBS})
install(TARGETS interceptor DESTINATION bin)

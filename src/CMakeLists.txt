set(src
  Main.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

subdirs(core)
subdirs(http)
subdirs(cache)
subdirs(backend)
subdirs(utils)
subdirs(socket)

set(LIBS
  interceptor_core
  interceptor_http
  interceptor_cache
  interceptor_backend
  interceptor_utils
  interceptor_socket
  ${LIBS}
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

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
subdirs(gateway)
subdirs(utils)
subdirs(socket)
subdirs(common)
subdirs(modules)

set(LIBS
  interceptor_core
  interceptor_cache
  interceptor_backend
  interceptor_http
  interceptor_gateway
  interceptor_socket
  interceptor_common
  interceptor_utils
  interceptor_modules
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

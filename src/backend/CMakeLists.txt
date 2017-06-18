set(libbackend_src
  BackendConnector.cpp
  BackendsPool.cpp
	)

INCLUDE_DIRECTORIES(
  "."
  )

add_library(interceptor_backend STATIC ${libbackend_src})

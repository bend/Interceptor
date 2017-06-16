set(libbackend_src
  BackendConnector.cpp
	)

INCLUDE_DIRECTORIES(
  "."
  )

add_library(interceptor_backend STATIC ${libbackend_src})

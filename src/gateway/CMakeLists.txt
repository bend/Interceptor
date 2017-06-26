set(libgateway_src
  GatewayHandler.cpp
  AbstractGateway.cpp
  BackendGateway.cpp
	)

INCLUDE_DIRECTORIES(
  "."
  )

add_library(interceptor_gateway STATIC ${libgateway_src})

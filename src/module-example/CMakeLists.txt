set(libhello_src
  HelloWorldModule.cpp
  ../common/Buffer.cpp
  ../utils/Logger.cpp
	)

INCLUDE_DIRECTORIES(
  "."
  )

add_library(interceptor_hello SHARED ${libhello_src})

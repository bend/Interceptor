set(libcore_src
  Interceptor.cpp
  InterceptorSession.cpp
  Config.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_core STATIC ${libcore_src})

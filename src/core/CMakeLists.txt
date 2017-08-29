set(libcore_src
  Server.cpp
  Session.cpp
  Config.cpp
  SessionTypeDetector.cpp
  SessionConnection.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_core STATIC ${libcore_src})

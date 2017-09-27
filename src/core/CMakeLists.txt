set(libcore_src
  Server.cpp
  Session.cpp
  Config.cpp
  SessionTypeDetector.cpp
  SessionConnection.cpp
)

if(ENABLE_SSL)
  set(libcore_src
    ${libcore_src}
    SSLSessionConnection.cpp
  )
endif(ENABLE_SSL)



INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_core STATIC ${libcore_src})

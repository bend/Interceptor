set(libutils_src
  Logger.cpp
  Server.cpp
  FileUtils.cpp
  
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_utils STATIC ${libutils_src})

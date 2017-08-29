set(libutils_src
  Logger.cpp
  ServerInfo.cpp
  FileUtils.cpp
  StringUtils.cpp 
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_utils STATIC ${libutils_src})

set(libmodules_src
  AbstractModule.cpp
  ModulesLoader.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_modules STATIC ${libmodules_src})

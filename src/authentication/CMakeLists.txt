set(libauth_src
  AuthenticationLoader.cpp
  AbstractAuthentication.cpp
  BasicAuthentication.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_auth STATIC ${libauth_src})

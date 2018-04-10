set(libhttp2_src
  Http2SessionHandler.cpp
  Http2SessionUpgrader.cpp
  Http2UpgradeReply.cpp
)

INCLUDE_DIRECTORIES(
  "."
)

add_library(interceptor_http2 STATIC ${libhttp2_src})


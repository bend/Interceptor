set(libcommon_src
  AbstractSessionHandler.cpp
  FileBuffer.cpp
  Buffer.cpp
	)

INCLUDE_DIRECTORIES(
  "."
  )

add_library(interceptor_common STATIC ${libcommon_src})

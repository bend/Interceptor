set(libcommon_src
  AbstractSessionHandler.cpp
  FileBuffer.cpp
	)

INCLUDE_DIRECTORIES(
  "."
  )

add_library(interceptor_common STATIC ${libcommon_src})

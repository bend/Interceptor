if(ENABLE_LOCAL_CACHE)
  set(libcache_src
	CacheHandler.cpp
	FileMetadataDatabase.cpp
	FileDatabase.cpp
	CacheMonitor.cpp
	Subject.cpp
	CacheListener.cpp
	MonitorListener.cpp

	)
else(ENABLE_LOCAL_CACHE)
  set(libcache_src BasicCacheHandler.cpp)
endif(ENABLE_LOCAL_CACHE)


INCLUDE_DIRECTORIES(
  "."
  )

add_library(interceptor_cache STATIC ${libcache_src})

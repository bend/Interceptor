#----------------------------------------------------------------
# Generated CMake target import file for configuration "DEBUG".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "interceptor_http" for configuration "DEBUG"
set_property(TARGET interceptor_http APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(interceptor_http PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libinterceptor_http.so"
  IMPORTED_SONAME_DEBUG "libinterceptor_http.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS interceptor_http )
list(APPEND _IMPORT_CHECK_FILES_FOR_interceptor_http "${_IMPORT_PREFIX}/lib/libinterceptor_http.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

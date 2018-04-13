cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0015 NEW)
project(Interceptor)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

option(CPACK_ENABLED OFF "Whether packaging with CPack is enabled (and the git version is retrieved")

set(ENABLE_GZIP ON CACHE BOOL "ENABLE GZIP") # Configurable by user
set(DEBUG_LOGGING OFF CACHE BOOL "DEBUG_LOGGING") # Configurable by user
set(ENABLE_LOCAL_CACHE ON CACHE BOOL "ENABLE_LOCAL_CACHE") # Configurable by user
set(DUMP_NETWORK OFF CACHE BOOL "DUMP_NETWORK") # Configurable by user
set(ENABLE_SSL ON CACHE BOOL "ENABLE_SSL") # Configurable by user

include(./cmake/FindFam.cmake)

if(NOT FAM_FOUND)
  MESSAGE(WARNING "** libfam not found, disabling local cache **")
  set(ENABLE_LOCAL_CACHE OFF CACHE BOOL "ENABLE_LOCAL_CACHE" FORCE) # Configurable by user
endif(NOT FAM_FOUND)

if(CPACK_ENABLED)
  include(GetGitRevisionDescription)
  get_git_head_revision(GIT_REFSPEC GIT_REVISION)

  if(GIT_REVISION)
    message(STATUS "Building git version ${GIT_REVISION}")
  endif()
endif(CPACK_ENABLED)

option(Boost_USE_MULTITHREADED ON "Use multithreaded boost")
  
find_package(Boost REQUIRED COMPONENTS date_time filesystem program_options system signals )

if(ENABLE_GZIP)
  find_package(ZLIB REQUIRED)
endif(ENABLE_GZIP)

if(NOT ZLIB_FOUND)
  MESSAGE(WARNING "** zlib not found, disabling gzip compression **")
  set(ENABLE_GZIP OFF CACHE BOOL "ENABLE_GZIP" FORCE) # Configurable by user
endif(NOT ZLIB_FOUND)

if(ENABLE_SSL)
  find_package(OpenSSL)
endif(ENABLE_SSL)

if(NOT OPENSSL_FOUND) 
  MESSAGE(WARNING "** OpenSSL not found, disabling SSL support **")
  set(ENABLE_SSL OFF CACHE BOOL "ENABLE_SSL" FORCE) # Configurable by user
endif(NOT OPENSSL_FOUND)

execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE INTERCEPTOR_GIT_COMMIT_ID
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

add_definitions(-DINTERCEPTOR_GIT_COMMIT_ID="${INTERCEPTOR_GIT_COMMIT_ID}")

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(
	${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
)

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
string(TIMESTAMP BUILD_DATE UTC)

add_definitions(-DINTERCEPTOR_BUILD_DATE="${BUILD_DATE}")

set(LIBS
    ${Boost_LIBRARIES}
    pthread
	dl
  )

if(ENABLE_GZIP)
  set(LIBS ${LIBS} ${ZLIB_LIBRARIES})
  include_directories(
	${ZLIB_INCLUDE_DIRS}
  )
endif(ENABLE_GZIP)

if(ENABLE_SSL)
  set(LIBS ${LIBS} ${OPENSSL_LIBRARIES})
  include_directories(
	${OPENSSL_INCLUDE_DIRS}
    )
endif(ENABLE_SSL)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fext-numeric-literals -Wall -Werror=return-type")

subdirs(src)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CPACK_ENABLED)
  set(CPACK_PACKAGE_NAME "interceptor")
	set(CPACK_PACKAGE_VENDOR "Bend")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "interceptor")
	set(CPACK_PACKAGE_FILE_NAME Interceptor-${GIT_REVISION})
	set(CPACK_GENERATOR "TGZ")
	set(CPACK_STRIP_FILES "1")
	include(CPack)
endif(CPACK_ENABLED)

configure_file(vars.h.in vars.h)

add_test(NAME http_normal_behavior COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_valid.py)
add_test(NAME http_invalid_behavior COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_invalid.py)
add_test(NAME http_load COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_load.py)
add_test(NAME backend_normal_behavior COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_validbackend.py)
add_test(NAME backend_invalid_behavior COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_invalidbackend.py)
add_test(NAME backend_load COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_loadbackend.py)

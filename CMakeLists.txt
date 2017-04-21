cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)
project(Interceptor)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

option(CPACK_ENABLED OFF "Whether packaging with CPack is enabled (and the git version is retrieved")

set(ENABLE_GZIP ON CACHE BOOL "ENABLE GZIP") # Configurable by user
set(DEBUG_LOGGING OFF CACHE BOOL "DEBUG_LOGGING") # Configurable by user
set(ENABLE_LOCAL_CACHE ON CACHE BOOL "ENABLE_LOCAL_CACHE") # Configurable by user

include(./cmake/FindFam.cmake)


if(NOT FAM_FOUND)
  MESSAGE(WARNING "** libfam not found, disabling local cache **")
  set(ENABLE_LOCAL_CACHE OFF)
endif(NOT FAM_FOUND)

if(CPACK_ENABLED)
  include(GetGitRevisionDescription)
  get_git_head_revision(GIT_REFSPEC GIT_REVISION)

  if(GIT_REVISION)
    message(STATUS "Building git version ${GIT_REVISION}")
  endif()
endif(CPACK_ENABLED)

option(Boost_USE_MULTITHREADED ON "Use multithreaded boost")
  
find_package(Boost REQUIRED COMPONENTS date_time filesystem program_options system regex thread)

execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE INTERCEPTOR_GIT_COMMIT_ID
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

add_definitions(-DINTERCEPTOR_GIT_COMMIT_ID="${INTERCEPTOR_GIT_COMMIT_ID}")

link_directories(
)

include_directories(
	${Boost_INCLUDE_DIRS}
	/opt/local/include
	${CMAKE_CURRENT_BINARY_DIR}
)


string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)

set(LIBS
    ${Boost_LIBRARIES}
    pthread
  )

if(ENABLE_LOCAL_CACHE)
  include_directories(
	${FAM_INCLUDE_DIR}
	)
  set(LIBS ${LIBS} ${FAM_LIBRARIES})
endif(ENABLE_LOCAL_CACHE)


if(ENABLE_GZIP)
  set(LIBS ${LIBS} z)
endif(ENABLE_GZIP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fext-numeric-literals -Wall -Werror=return-type")

subdirs(src)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CPACK_ENABLED)
  set(CPACK_PACKAGE_NAME "Interceptor")
	set(CPACK_PACKAGE_VENDOR "Ben")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Interceptor")
	set(CPACK_PACKAGE_FILE_NAME Interceptor-${GIT_REVISION})
	set(CPACK_GENERATOR "TGZ")
	set(CPACK_STRIP_FILES "1")

	include(CPack)
endif(CPACK_ENABLED)

configure_file(vars.h.in vars.h)

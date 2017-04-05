cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)
project(Interceptor)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

option(CPACK_ENABLED OFF "Whether packaging with CPack is enabled (and the git version is retrieved")

if(CPACK_ENABLED)
  include(GetGitRevisionDescription)
  get_git_head_revision(GIT_REFSPEC GIT_REVISION)

  if(GIT_REVISION)
    message(STATUS "Building git version ${GIT_REVISION}")
  endif()
endif(CPACK_ENABLED)

option(USE_STATIC_LIBS OFF "Use static libs")
set(Boost_USE_STATIC_LIBS ${USE_STATIC_LIBS})
option(Boost_USE_MULTITHREADED ON "Use multithreaded boost")
option(Boost_USE_STATIC_RUNTIME ON "Use static runtime for boost")

  
find_package(Boost REQUIRED COMPONENTS date_time filesystem program_options system regex)

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
)

string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)

set(LIBS
    ${Boost_LIBRARIES}
    pthread
	z
  )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror=return-type")

subdirs(src)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(CPACK_ENABLED)
  set(CPACK_PACKAGE_NAME "Interceptor")
	set(CPACK_PACKAGE_VENDOR "Ben")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Interceptor")
	set(CPACK_PACKAGE_FILE_NAME BlooLoc-software-${GIT_REVISION})
	set(CPACK_GENERATOR "TGZ")
	set(CPACK_STRIP_FILES "1")

	include(CPack)
endif(CPACK_ENABLED)

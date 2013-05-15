cmake_minimum_required (VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(gspcnet)
string (TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

include (SDPAFindBoost)
find_package(Threads REQUIRED QUIET)
find_package(UTIL REQUIRED QUIET)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${UTIL_INCLUDE_DIR})

add_subdirectory(gspc)
add_subdirectory(bin)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()

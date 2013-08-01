cmake_minimum_required (VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(gspcctl)
string (TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

include (SDPAFindBoost)
find_package(Threads REQUIRED QUIET)
find_package(UTIL REQUIRED QUIET)
find_package(GSPC_RIF REQUIRED QUIET)
include_directories (${GSPC_RIF_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${UTIL_INCLUDE_DIR})

add_subdirectory(gspc)
add_subdirectory(bin)
add_subdirectory(scripts)
add_subdirectory(doc)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()

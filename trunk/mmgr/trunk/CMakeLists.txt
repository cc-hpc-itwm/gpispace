# -*- mode: cmake; -*-
cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(libmmgr)

#set(PROJECT_DESCRIPTION "memory manager basic layer")
#set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
#set(PROJECT_COPYRIGHT   "Copyright 2009 Fraunhofer ITWM")
#set(PROJECT_CONTACT     "Mirko Rahn <mirko.rahn@itwm.fraunhofer.de>")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_FLAGS "-std=c99 -Wall -Werror")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -ggdb")
set(CMAKE_C_FLAGS_PROFILE "-O0 -g -pg")


# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

#link_directories(${CMAKE_LIBRARY_PATH})
#link_directories(${${PROJECT_NAME}_BINARY_DIR})

# Reflect the package structure
add_subdirectory(mmgr)
add_subdirectory(app)
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

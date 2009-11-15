# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(libfvm-pc)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 0)
set(V_MINOR 1)
set(V_PATCH 0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

#include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    if (${CMAKE_BUILD_TYPE} MATCHES "Release")
      add_definitions("-DNDEBUG")
    endif (${CMAKE_BUILD_TYPE} MATCHES "Release")

    set(FHG_PROFILE_FLAGS "-pg -fprofile-arcs -ftest-coverage")

    set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Werror")

    set(CMAKE_CXX_FLAGS_RELEASE "-O3 ")

    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wreturn-type -Wshadow")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-variable -Wunused-parameter")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-function -Wunused")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")
  endif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")

  # TODO: we need to check the compiler here, gcc does not know about those flags, is this The Right Thing To Do (TM)?
  if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "-wd383 -wd981")
  endif (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})

message(STATUS "CXXFLAGS: ${CMAKE_CXX_FLAGS}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: default (not-set)")
else ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
  string(TOUPPER ${CMAKE_BUILD_TYPE} tmp)
  message(STATUS "additional CXXFLAGS: ${CMAKE_CXX_FLAGS_${tmp}}")
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "")

add_definitions(-DSHMEM -DDEBUGALLOC -DDEBUG -DDEBUGCOMM  -DSHMEM -DUSE_STL_TR1)
if (MAX_SHMEM_SIZE)
else (MAX_SHMEM_SIZE)
  set (MAX_SHMEM_SIZE 536870912) # 512 MB
endif (MAX_SHMEM_SIZE)
add_definitions(-DMAX_SHMEM_SIZE=${MAX_SHMEM_SIZE})

# Reflect the package structure
add_subdirectory(fvm)
add_subdirectory(fvm-pc)

include(CTest)
add_subdirectory(tests)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/fvm.cfg.tpl
              ${CMAKE_CURRENT_SOURCE_DIR}/etc/shmemkey.tpl
              ${CMAKE_CURRENT_SOURCE_DIR}/etc/keyfile.tpl DESTINATION etc/)

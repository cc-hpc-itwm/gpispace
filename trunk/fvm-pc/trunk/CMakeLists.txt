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

option (WITH_FAKE_PC "Build with fake virtual machine interface library" ON)
set (FVM_FVM_SIZE 52428800 CACHE STRING "Default FVM size")
set (FVM_SHM_SIZE 52428800 CACHE STRING "Default SHM size")
set (FVM_MSQ_FILE "/p/herc/itwm/hpc/soft/sdpa/fvm-pc/etc/keyfile"  CACHE STRING "Default MSQ file")
set (FVM_SHM_FILE "/p/herc/itwm/hpc/soft/sdpa/fvm-pc/etc/shmemkey" CACHE STRING "Default SHM file")

message(STATUS "FVM config: shmsize=${FVM_SHM_SIZE} fvmsize=${FVM_FVM_SIZE}")
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

add_definitions(-DSHMEM -DDEBUGALLOC -DDEBUG -DDEBUGCOMM  -DSHMEM)
if (MAX_SHMEM_SIZE)
else (MAX_SHMEM_SIZE)
  set (MAX_SHMEM_SIZE 536870912) # 512 MB
endif (MAX_SHMEM_SIZE)
if (MAX_FVM_SIZE)
else (MAX_FVM_SIZE)
  set (MAX_FVM_SIZE 536870912) # 512 MB
endif (MAX_FVM_SIZE)
add_definitions(-DMAX_SHMEM_SIZE=${MAX_SHMEM_SIZE})
add_definitions(-DMAX_FVM_SIZE=${MAX_FVM_SIZE})

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.36" "1.36.0" "1.37" "1.37.0" "1.40" "1.40.0")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system filesystem serialization program_options)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

include (FindMMGR)
if (MMGR_FOUND)
  include_directories(${MMGR_INCLUDE_DIR})
else (MMGR_FOUND)
  message (FATAL_ERROR "Sorry, MemoryManager library is required")
endif (MMGR_FOUND)
message(STATUS "MMGR: -I${MMGR_INCLUDE_DIR} -L${MMGR_LIBRARY_DIR} -l${MMGR_LIBRARY}")

# Reflect the package structure
add_subdirectory(fvm)
add_subdirectory(fvm-pc)

include(CTest)
add_subdirectory(tests)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/etc/fvm.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/etc/fvm.cfg)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/shmemkey.tpl
              ${CMAKE_CURRENT_SOURCE_DIR}/etc/keyfile.tpl
              ${CMAKE_CURRENT_BINARY_DIR}/etc/fvm.cfg
        DESTINATION etc/
       )

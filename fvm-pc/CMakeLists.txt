# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(libfvm-pc)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

option (ENABLE_DEPRECATED_FVM_PC "Build the (deprecated) fvm pc library" ON)
option (ENABLE_FAKE_PC "Build with fake virtual machine interface library" ON)
set (FVM_FVM_SIZE "3221225472" CACHE STRING "Default virtual machine size")
set (FVM_SHM_SIZE "3221225472" CACHE STRING "Default shared memory segment size")
set (FVM_FAKE_FVM_SIZE "52428800" CACHE STRING "Default fake virtual machine size")
set (FVM_FAKE_SHM_SIZE "52428800" CACHE STRING "Default fake shared memory segment size")
set (FVM_MSQ_FILE "${CMAKE_INSTALL_PREFIX}/etc/gpi/key"  CACHE STRING "Default MSQ file")
set (FVM_SHM_FILE "${CMAKE_INSTALL_PREFIX}/etc/gpi/shm" CACHE STRING "Default SHM file")

message(STATUS "FVM config: shmsize=${FVM_SHM_SIZE} fvmsize=${FVM_FVM_SIZE}")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
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

add_definitions(-DSHMEM -DNDEBUGALLOC -DNDEBUG -DNDEBUGCOMM -DNDEBUGMSG)

if (ENABLE_FAKE_PC)
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
endif (ENABLE_FAKE_PC)

## check for boost
include(SDPAFindBoost)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/pnet/src/we)

find_package (FhgLog REQUIRED)
include_directories (${FhgLog_INCLUDE_DIR})

find_package(UTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

# Reflect the package structure
add_subdirectory(fvm)
add_subdirectory(fvm-pc)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/etc/fvm.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/etc/fvm.cfg @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/etc/fvm.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/etc/fvm.fake.cfg @ONLY)

# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/keyfile
#   DESTINATION etc/gpi
#   COMPONENT configurations
#   RENAME key
#   )
# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/shmemkey
#   DESTINATION etc/gpi
#   COMPONENT configurations
#   RENAME shm
#   )

# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/etc/fvm.cfg
#   DESTINATION etc/gpi
#   COMPONENT configurations
#   RENAME sdpa-gpi.cfg.tpl)

# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/etc/fvm.fake.cfg
#   DESTINATION etc/gpi
#   COMPONENT configurations
#   RENAME sdpa-gpi.fake.cfg.tpl)

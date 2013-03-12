# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(libfhgcom)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

option(USE_STL_TR1
 "Use tr1 from stl"
 0)

set(FHG_COM_NS_UUID "c9fe00cb-d9f7-432e-9235-66b7929b6e2a" CACHE STRING "The unique namespace uuid for this installation")

# don't forget to add the configuration option inc libfhglogConfig.cmake.in

set(PROJECT_DESCRIPTION "FHG Communication Framework")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# threads
include(FindThreads)
if(CMAKE_USE_PTHREADS_INIT)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

include(SDPAFindBoost)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

## find zlib
find_package(ZLIB REQUIRED)
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
message(STATUS "ZLib: Inc:${ZLIB_INCLUDE_DIR} Lib:${ZLIB_LIBRARIES}")

# logging
find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

include(FindUTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fhgcom/ns_uuid.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/fhgcom/ns_uuid.cpp)

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(TargetDoc OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "CXXFLAGS: ${CMAKE_CXX_FLAGS}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: default (not-set)")
else ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
  string(TOUPPER ${CMAKE_BUILD_TYPE} tmp)
  message(STATUS "additional CXXFLAGS: ${CMAKE_CXX_FLAGS_${tmp}}")
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "")

# Reflect the package structure
add_subdirectory(fhgcom)
add_subdirectory(bin)
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

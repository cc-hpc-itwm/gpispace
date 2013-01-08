# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

function (add_we_sources)
  foreach (source ${ARGN})
    set (WE_SOURCES ${WE_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/${source}
      CACHE INTERNAL "we_sources" FORCE
    )
  endforeach()
endfunction()

project(libwe)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

set(PROJECT_DESCRIPTION "Workflow Engine Header only library")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")

set (WE_NUM_EXTRACTORS "1" CACHE STRING "number of extractors")
set (WE_NUM_INJECTORS  "1" CACHE STRING "number of injectors")

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
    set(HAVE_PTHREAD_H 1)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

include(SDPAFindBoost)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

## fhglog
find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

include(FindUTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/we/we-config.hpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/we/we-config.hpp
  DESTINATION include/we
  COMPONENT headers)
add_definitions(-DHAVE_CONFIG_H)

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

# add some files to the installation target
#INSTALL(FILES INSTALL README COPYING ChangeLog NEWS
#  DESTINATION share/doc/${PROJECT_NAME}
#  COMPONENT documentation
#)

# Reflect the package structure
add_subdirectory(we)
message(STATUS "WE_LIB = ${WE_LIBRARIES}")
add_subdirectory(apps)

# CTest
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

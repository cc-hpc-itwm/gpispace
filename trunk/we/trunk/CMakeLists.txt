# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

OPTION(WE_PRECOMPILE "precompile" NO)

set (WE_PRECOMPILE_LIBRARIES CACHE INTERNAL "we_precompile" FORCE)

function(add_we_precompile_lib libname srcs)
  if (WE_PRECOMPILE)
    ADD_LIBRARY(${libname} ${srcs})
    set (WE_PRECOMPILE_LIBRARIES ${WE_PRECOMPILE_LIBRARIES}
         ${libname}
         CACHE INTERNAL "${libname}" FORCE
        )
  endif (WE_PRECOMPILE)
endfunction(add_we_precompile_lib)

if (WE_PRECOMPILE)
  add_definitions (-DWE_PRECOMPILE)
endif (WE_PRECOMPILE)


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

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.36.0" "1.36")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization program_options filesystem test_exec_monitor)
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
INSTALL(FILES INSTALL README COPYING ChangeLog NEWS
  DESTINATION share/doc/${PROJECT_NAME}
  COMPONENT documentation
)

# Reflect the package structure
add_subdirectory(we)
add_subdirectory(apps)

# CTest
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)


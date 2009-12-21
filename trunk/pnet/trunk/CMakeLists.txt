# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(pnet)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 0)
set(V_MINOR 1)
set(V_PATCH 0)

option(BUILD_WITH_WERROR "build with -Werror" Yes)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    if (${CMAKE_BUILD_TYPE} MATCHES "Release")
      add_definitions("-DNDEBUG")
    endif (${CMAKE_BUILD_TYPE} MATCHES "Release")

    set(FHG_PROFILE_FLAGS "-pg -fprofile-arcs -ftest-coverage")

    set(CMAKE_CXX_FLAGS "-W -Wall -Wextra")
    if (BUILD_WITH_WERROR)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    endif (BUILD_WITH_WERROR)

    set(CMAKE_CXX_FLAGS_RELEASE "-O3")

    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -fno-omit-frame-pointer")
    if (${CMAKE_CXX_COMPILER_MAJOR} GREATER 3)
      if (${CMAKE_CXX_COMPILER_MINOR} GREATER 3)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pedantic -Wreturn-type -Wshadow")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-variable -Wunused-parameter")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-function -Wunused")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")
        if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
          message(STATUS "Additional warnings for Debug-Build activated (gcc >= 4.3 detected)")
        endif (${CMAKE_BUILD_TYPE} MATCHES "Debug")
      elseif (${CMAKE_CXX_COMPILER_MINOR} GREATER 0)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wreturn-type")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")
      endif (${CMAKE_CXX_COMPILER_MINOR} GREATER 3)
    endif (${CMAKE_CXX_COMPILER_MAJOR} GREATER 3)
  endif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")

  # TODO: we need to check the compiler here, gcc does not know about those flags, is this The Right Thing To Do (TM)?
  if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "-wd383 -wd981")
  endif (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.36.0" "1.36")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization program_options)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

include(FindFhgLog)
if (FHGLOG_FOUND)
  message(STATUS "Found FhgLog: Inc:${FHGLOG_INCLUDE_DIR} Lib:${FHGLOG_LIBRARY}")
  include_directories(${FHGLOG_INCLUDE_DIR})
  set(SDPA_HAVE_FHGLOG 1)
else (FHGLOG_FOUND)
  message(FATAL_ERROR "Sorry, FhgLog is required!")
endif(FHGLOG_FOUND)

if (FHGLOG_DYNAMIC)
  set(FHGLOG_LIBRARY "${FHGLOG_HOME}/lib/libfhglog.so")
  message(STATUS "setting FhgLog library to: ${FHGLOG_LIBRARY}")
endif (FHGLOG_DYNAMIC)

message(STATUS "*** Fixme: FVM includes...")
set(FVM_PC_INCLUDE_DIR "${FVM_PC_HOME}/include")

if (FVM_PC_STATIC)
  set(FVM_PC_LIBRARY "${FVM_PC_HOME}/lib/libfvm-pc.a")
else (FVM_PC_STATIC)
  set(FVM_PC_LIBRARY "${FVM_PC_HOME}/lib/libfvm-pc.so")
endif (FVM_PC_STATIC)

include_directories(${FVM_PC_INCLUDE_DIR})

message(STATUS "*** Fixme: SDPA includes...")
include_directories(${SDPA_INCLUDE_DIR})

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

# Reflect the package structure
add_subdirectory(pnet)

include(CTest)
add_subdirectory(tests)

install(FILES README DESTINATION share/pnet)

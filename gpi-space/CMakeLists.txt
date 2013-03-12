cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project (gpi-space)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

OPTION(ENABLE_REAL_GPI "Provide api for the *real* GPI" ON)
OPTION(ENABLE_COMPATIBILITY_API "Provide a transitional api compatible with the old fvm-pc" ON)

set(PROJECT_DESCRIPTION "GPI-Space interface")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "GPISpace Developer Team <sdpa-dev@itwm.fraunhofer.de>")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (QUIET_FIND_PACKAGE QUIET)

if(ENABLE_REAL_GPI)
  find_package (IBVerbs ${QUIET_FIND_PACKAGE})
  if (IBverbs_FOUND)
    include_directories(SYSTEM ${IBverbs_INCLUDE_DIR})

    set (GPI_HOME ${CMAKE_CURRENT_SOURCE_DIR}/ext)
    set (GPI_INCLUDE_DIR ${GPI_HOME}/include)
    set (GPI_LIBRARIES ${GPI_LIBRARIES} ${GPI_HOME}/lib64/libGPI.a)
    set (GPI_LIBRARIES ${GPI_LIBRARIES} ${IBverbs_LIBRARY})

    # check if we can actually use the GPI version in ext
    set(CMAKE_REQUIRED_INCLUDES ${GPI_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${GPI_LIBRARIES})
    include(CheckCSourceRuns)
    CHECK_C_SOURCE_RUNS(
      "
#include <stdio.h>
#include <GPI.h>
int main ()
{
   printf(\"%f\", getVersionGPI());
   return 0;
}"
    REAL_GPI_CHECK)
    if("${REAL_GPI_CHECK}" EQUAL 1)
      add_definitions(-DENABLE_REAL_GPI=1)
    else("${REAL_GPI_CHECK}" EQUAL 1)
      message(WARNING "Support for real-GPI requested but not available on your platform!")
      set(ENABLE_REAL_GPI OFF CACHE BOOL "Provide api for the *real* GPI" FORCE)
    endif("${REAL_GPI_CHECK}" EQUAL 1)
  else (IBverbs_FOUND)
    message(STATUS "${IBverbs_FOUND}")
    message(WARNING "Support for real-GPI requested but not available on your platform (ibverbs not installed)!")
    set(ENABLE_REAL_GPI OFF CACHE BOOL "Provide api for the *real* GPI" FORCE)
  endif (IBverbs_FOUND)
endif(ENABLE_REAL_GPI)

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# threads
include(FindThreads)

# header files
include(CheckIncludeFile)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
if (HAVE_SYS_TYPES_H)
else (HAVE_SYS_TYPES_H)
  message(FATAL_ERROR "sys/types.h is required for mode_t")
endif (HAVE_SYS_TYPES_H)

check_include_file(unistd.h HAVE_UNISTD_H)
if (HAVE_UNISTD_H)
else (HAVE_UNISTD_H)
  message(FATAL_ERROR "unistd.h is required for sysconf")
endif (HAVE_UNISTD_H)

# symbols
include(CheckSymbolExists)
check_symbol_exists(_SC_GETPW_R_SIZE_MAX unistd.h HAVE_GETPW_R_SIZE_MAX)
if (HAVE_GETPW_R_SIZE_MAX)
else (HAVE_GETPW_R_SIZE_MAX)
  message(FATAL_ERROR "_SC_GETPW_R_SIZE_MAX doesn't seem to be defined on this machine")
endif (HAVE_GETPW_R_SIZE_MAX)

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package (FhgLog REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${FhgLog_INCLUDE_DIR})

find_package (FhgCom REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${FhgCom_INCLUDE_DIR})

find_package (UTIL REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${UTIL_INCLUDE_DIR})

find_package (FhgKernel REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${FhgKernel_INCLUDE_DIR})

find_package (MMGR REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${MMGR_INCLUDE_DIR})

find_package (LibRT REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${LIBRT_INCLUDE_DIR})

find_package (ZMQ ${QUIET_FIND_PACKAGE})
if (ZMQ_FOUND)
  include_directories (${ZMQ_INCLUDE_DIR})
endif ()

find_package (CZMQ ${QUIET_FIND_PACKAGE})
if (CZMQ_FOUND)
  include_directories (SYSTEM ${CZMQ_INCLUDE_DIR})
endif()

add_subdirectory(gpi-space)
add_subdirectory(bin)

if (ENABLE_COMPATIBILITY_API)
  add_subdirectory(compat)
endif (ENABLE_COMPATIBILITY_API)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project (gpi-space)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

OPTION(ENABLE_REAL_GPI "Provide api for the *real* GPI" ON)

set(PROJECT_DESCRIPTION "GPI-Space interface")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_REAL_GPI)
  set (GPI_HOME ${CMAKE_CURRENT_SOURCE_DIR}/ext)
  set (GPI_INCLUDE_DIR ${GPI_HOME}/include)
  set (GPI_LIBRARIES ${GPI_LIBRARIES} ${GPI_HOME}/lib64/libGPI.a)
  set (GPI_LIBRARIES ${GPI_LIBRARIES} ${GPI_HOME}/lib64/libibverbs15.so)

  # check if we can actually use the GPI version in ext
  set(CMAKE_REQUIRED_INCLUDES ${GPI_INCLUDE_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${GPI_LIBRARIES})
  include(CheckCXXSourceRuns)
  CHECK_CXX_SOURCE_RUNS(
"
#include <iostream>
#include <GPI/GPI.h>
int main ()
{
   std::cout << getVersionGPI() << std::endl;
   return 0;
}"
REAL_GPI_CHECK)
  if("${REAL_GPI_CHECK}" EQUAL 1)
    add_definitions(-DENABLE_REAL_GPI=1)
  else("${REAL_GPI_CHECK}" EQUAL 1)
    message(WARNING "Support for real-GPI requested but not available on your platform!")
    set(ENABLE_REAL_GPI OFF CACHE BOOL "Provide api for the *real* GPI" FORCE)
  endif("${REAL_GPI_CHECK}" EQUAL 1)
endif(ENABLE_REAL_GPI)

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# threads
include(FindThreads)
if(CMAKE_USE_PTHREADS_INIT)
    set(HAVE_PTHREAD_H 1)
endif(CMAKE_USE_PTHREADS_INIT)

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

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.36.0" "1.36")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization program_options test_exec_monitor iostreams filesystem)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

find_package (IBVerbs REQUIRED)
include_directories(${IBVerbs_INCLUDE_DIR})

find_package (FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

find_package (FhgCom REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

find_package (UTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

find_package (MMGR REQUIRED)
include_directories(${MMGR_INCLUDE_DIR})

add_subdirectory(gpi-space)
add_subdirectory(bin)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project (gpi-space)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

set(PROJECT_DESCRIPTION "GPI-Space interface")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_INCLUDE_PATH})

set (GPI_PRIV_DIR  CACHE STRING "path to the priviledged GPI binary directory")
set (GPI_CONF_PATH
  "${CMAKE_INSTALL_PREFIX}/etc/gpi/sdpa-gpi.cfg"
  CACHE
  STRING "path to the default location of the config file"
  )

# TODO move into FindGPI.cmake
set (GPI_HOME ${CMAKE_CURRENT_SOURCE_DIR}/ext)
set (GPI_LIBRARIES ${GPI_LIBRARIES} ${GPI_HOME}/lib64/libibverbs15.so)
set (GPI_LIBRARIES ${GPI_LIBRARIES} ${GPI_HOME}/lib64/libGPI.a)
set (GPI_INCLUDE_DIR ${GPI_HOME}/include)

include_directories (${GPI_INCLUDE_DIR})

if (GPI_PRIV_DIR)
else (GPI_PRIV_DIR)
  message(FATAL_ERROR "please run cmake -DGPI_PRIV_DIR=... and set it to the priviledged directory of the gpi daemon")
endif(GPI_PRIV_DIR)

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
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization program_options test_exec_monitor iostreams)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

find_package (FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

find_package (MMGR REQUIRED)
include_directories(${MMGR_INCLUDE_DIR})

find_package (UTIL REQUIRED)
include_directories(${MMGR_INCLUDE_DIR})

add_subdirectory(gpi-space)
add_subdirectory(bin)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

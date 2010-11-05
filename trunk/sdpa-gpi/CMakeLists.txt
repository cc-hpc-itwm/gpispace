cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project (sdpa-gpi)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

set(V_MAJOR 0)
set(V_MINOR 1)
set(V_PATCH 0)

set(PROJECT_DESCRIPTION "SDPA GPI interface")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")
set(PROJECT_VERSION     "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(PROJECT_README      "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/README")
set(PROJECT_COPYING     "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/COPYING")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/include)
include_directories(${CMAKE_INCLUDE_PATH})

set (GPI_BINARY_DIR  CACHE STRING "path to the GPI binary directory")
set (GPI_CONFIG_PATH "${CMAKE_INSTALL_PREFIX}/etc/gpi/sdpa-gpi.cfg" CACHE STRING "path to the sdpa-gpi config file")
set (EXT_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/lib64)
set (EXT_LIBRARIES ${EXT_LIBRARIES} ${EXT_LIBRARY_DIR}/libibverbs12.so)
set (EXT_LIBRARIES ${EXT_LIBRARIES} ${EXT_LIBRARY_DIR}/libPv4dVM4.a)
set (EXT_LIBRARIES ${EXT_LIBRARIES} ${EXT_LIBRARY_DIR}/libmmgr.a)

if (GPI_BINARY_DIR)
else (GPI_BINARY_DIR)
  message(FATAL_ERROR "please define GPI_BINARY_DIR and set it to the priviledged gpid directory")
endif(GPI_BINARY_DIR)

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

find_package (MMGR REQUIRED)
include_directories(${MMGR_INCLUDE_DIR})

add_subdirectory(src)

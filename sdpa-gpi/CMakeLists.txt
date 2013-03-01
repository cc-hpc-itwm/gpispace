cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project (sdpa-gpi)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

set(PROJECT_DESCRIPTION "SDPA GPI interface")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/include)
include_directories(${CMAKE_INCLUDE_PATH})

set (EXT_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/lib64)
set (EXT_LIBRARIES ${EXT_LIBRARIES} ${EXT_LIBRARY_DIR}/libibverbs12.so)
set (EXT_LIBRARIES ${EXT_LIBRARIES} ${EXT_LIBRARY_DIR}/libPv4dVM4.a)

# TODO move into FindGPI.cmake
set (GPI_LIBRARIES ${GPI_LIBRARIES} ${EXT_LIBRARY_DIR}/libibverbs15.so)
set (GPI_LIBRARIES ${GPI_LIBRARIES} ${EXT_LIBRARY_DIR}/libGPI.a)

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
include_directories(${Boost_INCLUDE_DIRS})

find_package (FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

find_package (MMGR REQUIRED)
include_directories(${MMGR_INCLUDE_DIR})

find_package (UTIL REQUIRED)
include_directories(${MMGR_INCLUDE_DIR})

add_subdirectory(src)
add_subdirectory(bin)

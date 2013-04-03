cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project (mapreduce)

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(FVM REQUIRED QUIET)
include_directories(${FVM_INCLUDE_DIR})

find_package(FhgLog REQUIRED QUIET)
include_directories (${FhgLog_INCLUDE_DIR})

include_directories (${PROCESS_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

add_subdirectory(xml)
add_subdirectory (test)

cmake_minimum_required(VERSION 2.6.2)

project(process)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
message(STATUS "MODULE_PATH := ${CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

include(SDPAFindBoost)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(FVM REQUIRED)
include_directories(${FVM_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/pnet/src/we)

find_package(UTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

find_package(FhgLog REQUIRED)
include_directories (${FhgLog_INCLUDE_DIR})

add_library(exec_mod SHARED process.cpp exec_mod.cpp)
set_target_properties(exec_mod PROPERTIES OUTPUT_NAME exec)

set (hdrs
  process.hpp
)

add_library(process ${hdrs} process.cpp)
add_library(process.shared SHARED ${hdrs} process.cpp)
set_target_properties(process.shared PROPERTIES OUTPUT_NAME process)

set (xmls
  exec.xml
  )

install(FILES ${xmls}
  DESTINATION share/sdpa/xml/process
  COMPONENT examples)

install(FILES ${hdrs}
  DESTINATION include/process
  COMPONENT module-headers
)

install(TARGETS process.shared
  LIBRARY
  DESTINATION libexec/sdpa
  COMPONENT modules)

include (CTest)
if (BUILD_TESTING)
  add_subdirectory (test)
endif (BUILD_TESTING)

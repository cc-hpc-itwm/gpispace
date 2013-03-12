cmake_minimum_required(VERSION 2.6.2)

project(stresstest)

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

include_directories(${CMAKE_SOURCE_DIR}/we)

find_package(UTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

find_package(FhgLog REQUIRED)
include_directories (${FhgLog_INCLUDE_DIR})

add_library(stresstest_mod SHARED stresstest.cpp)
set_target_properties(stresstest_mod PROPERTIES OUTPUT_NAME stresstest)

set (xmls
  stress.xpnet
  stresstest.xpnet
  )

install(TARGETS stresstest_mod
  LIBRARY
  DESTINATION libexec/sdpa/apps/stresstest/modules
  COMPONENT modules)

install(FILES ${xmls}
  DESTINATION libexec/sdpa/apps/stresstest
  COMPONENT examples)

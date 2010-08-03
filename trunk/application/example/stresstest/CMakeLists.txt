cmake_minimum_required(VERSION 2.6.2)

project(stresstest)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
message(STATUS "MODULE_PATH := ${CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.36" "1.36.0" "1.37" "1.37.0" "1.40" "1.40.0")
set(Boost_FIND_QUIETLY false)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system filesystem serialization program_options)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

find_package(FVM REQUIRED)
include_directories(${FVM_INCLUDE_DIR})

find_package(WE REQUIRED)
include_directories(${WE_INCLUDE_DIR})

find_package(UTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

find_package(FhgLog REQUIRED)
include_directories (${FhgLog_INCLUDE_DIR})

add_library(stresstest_mod SHARED stresstest.cpp)
set_target_properties(stresstest_mod PROPERTIES OUTPUT_NAME stresstest)

install(TARGETS stresstest_mod DESTINATION libexec)

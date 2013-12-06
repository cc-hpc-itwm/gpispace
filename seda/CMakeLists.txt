# -*- mode: cmake; -*-
project(seda)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

if (V_MAJOR)
else (V_MAJOR)
  set(V_MAJOR 1)
  set(V_MINOR 4)
  set(V_PATCH 0)
endif (V_MAJOR)

###################################################################################################
# No user settings below
###################################################################################################

OPTION(ENABLE_LOGGING
  "Build the project with logging enabled"
  1)

OPTION(ENABLE_SEDA_COMM_LIB
  "Build communication library"
  OFF)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

## add doxygen support
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(TargetDoc.cmake OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

## logging
if (ENABLE_LOGGING)
  set(SEDA_ENABLE_LOGGING ${ENABLE_LOGGING})

  include_directories(${FhgLog_INCLUDE_DIR})
endif (ENABLE_LOGGING)

## check for threads

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

## find zlib
if( NOT WIN32)
  find_package(ZLIB REQUIRED QUIET)
  include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
endif( NOT WIN32)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/seda/seda-config.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libseda.pc.in   ${CMAKE_CURRENT_BINARY_DIR}/libseda.pc @ONLY)

# Make sure all subdirectories include correctly

# Reflect the package structure
add_subdirectory(seda)
if (ENABLE_SEDA_COMM_LIB)
  add_subdirectory(seda/comm)
endif ()

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

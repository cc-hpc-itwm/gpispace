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

OPTION(ENABLE_SEDA_COMM_LIB
  "Build communication library"
  OFF)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${FhgLog_INCLUDE_DIR})

## find zlib
if( NOT WIN32)
  find_package(ZLIB REQUIRED QUIET)
  include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
endif( NOT WIN32)

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

# -*- mode: cmake; -*-
project(seda)

###################################################################################################
# No user settings below
###################################################################################################

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${FhgLog_INCLUDE_DIR})
include_directories(${UTIL_INCLUDE_DIR})

## find zlib
if( NOT WIN32)
  find_package(ZLIB REQUIRED QUIET)
  include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
endif( NOT WIN32)

# Make sure all subdirectories include correctly

# Reflect the package structure
add_subdirectory(seda)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

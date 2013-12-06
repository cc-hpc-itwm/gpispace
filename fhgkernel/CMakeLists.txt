# -*- mode: cmake; -*-
# don't forget to add the configuration option inc libfhglogConfig.cmake.in

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

include_directories(${FhgLog_INCLUDE_DIR})

include_directories(${UTIL_INCLUDE_DIR})

find_package(HWLOC REQUIRED QUIET)
include_directories(SYSTEM ${HWLOC_INCLUDE_DIR})

find_package (NACL QUIET)
if (NACL_FOUND)
  if (ENABLE_LICENSE_CHECK)
    include_directories (${NACL_INCLUDE_DIR})
  endif()
else()
  set(ENABLE_LICENSE_CHECK CACHE BOOL OFF FORCE)
endif()

# Reflect the package structure
add_subdirectory(fhg)
add_subdirectory(bin)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

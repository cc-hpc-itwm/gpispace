project(libfhglog)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${UTIL_INCLUDE_DIR})

add_subdirectory(fhglog)
add_subdirectory(apps)

add_subdirectory(tests)

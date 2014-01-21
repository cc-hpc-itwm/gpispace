include_directories (${CMAKE_CURRENT_BINARY_DIR})
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

add_subdirectory (fhg)
add_subdirectory (bin)

  add_subdirectory (tests)

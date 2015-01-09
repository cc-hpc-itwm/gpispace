include_directories (${CMAKE_CURRENT_SOURCE_DIR})

include_directories (${UTIL_INCLUDE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

add_subdirectory (sdpa)
add_subdirectory (apps)
add_subdirectory (bin)

  add_subdirectory (tests)

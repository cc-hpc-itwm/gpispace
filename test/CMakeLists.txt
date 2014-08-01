include_directories (${CMAKE_SOURCE_DIR})
include_directories (${UTIL_INCLUDE_DIR})

add_library (test-utilities SHARED
  make.cpp
  scoped_nodefile_with_localhost.cpp
  scoped_state_directory.cpp
  shared_directory.cpp
  source_directory.cpp
)
target_link_libraries (test-utilities
  fhg-util
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
)

  find_package (HWLOC REQUIRED QUIET)

  set (prefixes
    die
    dir_build
    dir_state
    env
    error
    measured
    measurement
  )

  foreach (prefix ${prefixes})
    configure_file (${prefix}.sh.in ${prefix}.sh @ONLY)
  endforeach()

include_directories (${CMAKE_SOURCE_DIR})
include_directories (${UTIL_INCLUDE_DIR})

add_library (test-utilities SHARED
  scoped_nodefile_with_localhost.cpp
  scoped_state_directory.cpp
)
target_link_libraries (test-utilities
  fhg-util
  ${Boost_PROGRAM_OPTIONS_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARIES}
)

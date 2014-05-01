project (libfhglog)

include_directories (${FhgLog_INCLUDE_DIR})
include_directories (${UTIL_INCLUDE_DIR})

add_subdirectory (fhglog)
add_subdirectory (apps)

add_subdirectory (tests)

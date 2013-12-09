include_directories (${FhgLog_INCLUDE_DIR})

include_directories (${CMAKE_SOURCE_DIR}/pnet/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${UTIL_INCLUDE_DIR})

add_subdirectory(gspc)
add_subdirectory(bin)
add_subdirectory(libexec)
add_subdirectory(scripts)
add_subdirectory(doc)

add_subdirectory(tests)

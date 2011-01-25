include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/datastructures)

add_definitions ("-DNDEBUGALLOC")
add_definitions ("-DNDEBUGMSG")
add_definitions ("-DNDEBUG")
add_definitions ("-DNDEBUGCMSG")
add_definitions ("-DNDEBUGCOMM")
add_definitions ("-DSHMEM")
add_definitions ("-DUSE_STL_TR1")

add_subdirectory(datastructures)
add_subdirectory(fvm)

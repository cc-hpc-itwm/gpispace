include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/include/PSProMigIF")

find_package(HWLOC REQUIRED QUIET)

set(PSPROMIGIF_LIBRARY
  "${CMAKE_CURRENT_SOURCE_DIR}/ext/lib64/libPSProMigIF.a")

# TODO how to locate that one correctly without having the intel compiler loaded?
set(LIBIRC_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/ext/lib64/libirc.a")

set(LOG4CPP_LIBRARY
  "${CMAKE_CURRENT_SOURCE_DIR}/ext/lib64/liblog4cplus.a")

add_subdirectory(bin)
add_subdirectory(example)
add_subdirectory(plugin)

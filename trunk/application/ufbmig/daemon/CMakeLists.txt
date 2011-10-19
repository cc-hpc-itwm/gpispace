include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/include/PSProMigIF")

find_package(HWLOC REQUIRED)

set(PSPROMIGIF_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/ext/lib64/libPSProMigIF.a")
set(LIBIRC_LIBRARY "/m/soft/Intel/cce/11.1.064/lib/intel64/libirc.a")

add_subdirectory(bin)
add_subdirectory(example)
add_subdirectory(plugin)

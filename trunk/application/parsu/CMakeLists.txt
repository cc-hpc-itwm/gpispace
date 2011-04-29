configure_file (${CMAKE_CURRENT_SOURCE_DIR}/make.defs.in ${CMAKE_CURRENT_SOURCE_DIR}/make.defs @ONLY)

add_subdirectory(bin)
add_subdirectory(stack)
add_subdirectory(shrink)
add_subdirectory(op2)

# NEW STYLE, using seislib
add_subdirectory(diff)

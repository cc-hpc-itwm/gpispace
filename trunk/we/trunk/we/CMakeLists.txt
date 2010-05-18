# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
install(FILES ${hdrs} DESTINATION include/we)

add_subdirectory (concurrent)
add_subdirectory (container)
add_subdirectory (expr)
add_subdirectory (function)
add_subdirectory (mgmt)
add_subdirectory (serialize)
add_subdirectory (type)
add_subdirectory (util)

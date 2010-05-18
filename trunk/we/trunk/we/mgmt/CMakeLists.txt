# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
install(FILES ${hdrs} DESTINATION include/we/mgmt)

add_subdirectory (bits)
add_subdirectory (type)

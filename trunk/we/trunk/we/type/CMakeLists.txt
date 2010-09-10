# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
install(FILES ${hdrs} DESTINATION include/we/type)

add_subdirectory (bits)
add_subdirectory (literal)
add_subdirectory (value)
add_subdirectory (signature)

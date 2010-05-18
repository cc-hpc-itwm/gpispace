# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
install(FILES ${hdrs} DESTINATION include/we/type)

add_subdirectory (literal)
add_subdirectory (value)

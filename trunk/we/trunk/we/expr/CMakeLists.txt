# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
install(FILES ${hdrs} DESTINATION include/we/expr)

add_subdirectory (eval)
add_subdirectory (parse)
add_subdirectory (token)

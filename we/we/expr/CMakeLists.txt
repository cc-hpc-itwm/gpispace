# -*- mode: cmake; -*-
set (hdrs
  exception.hpp
  )

add_we_sources(exception.cpp)

install(FILES ${hdrs}
  DESTINATION include/we/expr
  COMPONENT headers)

add_subdirectory (parse)
add_subdirectory (eval)
add_subdirectory (token)

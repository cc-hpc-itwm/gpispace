# -*- mode: cmake; -*-
set (hdrs
  exception.hpp
  expr.hpp
  )
install(FILES ${hdrs}
  DESTINATION include/we/expr
  COMPONENT headers)

add_subdirectory (eval)
add_subdirectory (parse)
add_subdirectory (token)

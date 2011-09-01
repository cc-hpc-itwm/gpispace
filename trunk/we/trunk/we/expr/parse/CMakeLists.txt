# -*- mode: cmake; -*-
set (hdrs
  action.hpp
  node.hpp
  parser.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse
  COMPONENT headers)

add_subdirectory (simplify)
add_subdirectory (util)

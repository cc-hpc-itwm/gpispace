# -*- mode: cmake; -*-
set (hdrs
  action.hpp
  node.hpp
  parser.hpp
  )

set (srcs
  parser.cpp
  node.cpp
  action.cpp
)

add_library (we_expr_parse ${srcs})
add_we_library (we_expr_parse)

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse
  COMPONENT headers)

add_subdirectory (simplify)
add_subdirectory (util)

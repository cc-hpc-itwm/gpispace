# -*- mode: cmake; -*-
set (hdrs
  action.hpp
  node.hpp
  parser.hpp
  )

set (srcs
  parser.cpp
  node.cpp
)

add_library(we_expr_parser ${srcs})
add_we_library (we_expr_parser)

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse
  COMPONENT headers)

add_subdirectory (simplify)
add_subdirectory (util)

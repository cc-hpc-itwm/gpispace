# -*- mode: cmake; -*-
set (hdrs
  get_names.hpp
  )

set (srcs
  get_names.cpp
)

add_library(we_expr_parse_util ${srcs})
add_we_library (we_expr_parse_util)

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse/util
  COMPONENT headers)

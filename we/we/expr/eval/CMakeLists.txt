# -*- mode: cmake; -*-
set (hdrs
  context.hpp
  eval.hpp
  )

set (srcs
  eval.cpp
  context.cpp
)

add_library (we_expr_eval ${srcs})
add_we_library (we_expr_eval)

install(FILES ${hdrs}
  DESTINATION include/we/expr/eval
  COMPONENT headers)

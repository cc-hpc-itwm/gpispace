# -*- mode: cmake; -*-
set (hdrs
  assoc.hpp
  prec.hpp
  prop.hpp
  tokenizer.hpp
  type.hpp
  )

set (srcs
  type.cpp
  )

add_library(we_expr_token ${srcs})
add_we_library (we_expr_token)

install(FILES ${hdrs}
  DESTINATION include/we/expr/token
  COMPONENT headers)

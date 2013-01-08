# -*- mode: cmake; -*-
set (hdrs
  context.hpp
  eval.hpp
  )

add_we_sources (
  eval.cpp
  context.cpp
)

install(FILES ${hdrs}
  DESTINATION include/we/expr/eval
  COMPONENT headers)

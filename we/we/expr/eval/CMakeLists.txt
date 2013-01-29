# -*- mode: cmake; -*-
set (hdrs
  context.hpp
  eval.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/eval
  COMPONENT headers)

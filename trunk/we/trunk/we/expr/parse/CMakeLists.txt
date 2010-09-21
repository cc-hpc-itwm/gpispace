# -*- mode: cmake; -*-
set (hdrs
  action.hpp
  node.hpp
  parser.hpp
  position.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse
  COMPONENT headers)

# -*- mode: cmake; -*-

set (hdrs
  is_associated.hpp
  merge_expressions.hpp
  merge_places.hpp
  option.hpp
  simple_pipe_elimination.hpp
)

install(FILES ${hdrs}
  DESTINATION include/we/type/bits/transition/optimize
  COMPONENT headers)

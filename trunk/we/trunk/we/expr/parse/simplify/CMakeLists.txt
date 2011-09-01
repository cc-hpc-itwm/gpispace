# -*- mode: cmake; -*-
set (hdrs
  dead_code_elimination_pass.hpp
  expression_list.hpp
  numbering_and_propagation_pass.hpp
  simplify.hpp
  ssa_tree.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse/simplify
  COMPONENT headers)

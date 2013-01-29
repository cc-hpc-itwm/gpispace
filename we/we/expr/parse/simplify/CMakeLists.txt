#bernd.loerwald@itwm.fraunhofer.de

set (hdrs
  constant_propagation.hpp
  copy_propagation.hpp
  dead_code_elimination.hpp
  expression_list.hpp
  simplify.hpp
  util.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse/simplify
  COMPONENT headers)

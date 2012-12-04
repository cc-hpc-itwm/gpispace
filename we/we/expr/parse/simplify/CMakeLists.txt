#bernd.loerwald@itwm.fraunhofer.de

set (hdrs
  constant_propagation.hpp
  copy_propagation.hpp
  dead_code_elimination.hpp
  expression_list.hpp
  simplify.hpp
  util.hpp
  )

set (sources
  constant_propagation.cpp
  copy_propagation.cpp
  dead_code_elimination.cpp
  expression_list.cpp
  simplify.cpp
  util.cpp
  )

add_library (we_expr_parse_simplify ${sources})
add_we_library (we_expr_parse_simplify)

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse/simplify
  COMPONENT headers)

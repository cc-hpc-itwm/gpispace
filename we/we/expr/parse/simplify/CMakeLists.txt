#bernd.loerwald@itwm.fraunhofer.de

set (hdrs
  constant_propagation.hpp
  copy_propagation.hpp
  dead_code_elimination.hpp
  expression_list.hpp
  simplify.hpp
  util.hpp
  )

add_we_sources (
  constant_propagation.cpp
  copy_propagation.cpp
  dead_code_elimination.cpp
  expression_list.cpp
  simplify.cpp
  util.cpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse/simplify
  COMPONENT headers)

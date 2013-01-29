# -*- mode: cmake; -*-
set (hdrs
  action.hpp
  node.hpp
  parser.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse
  COMPONENT headers)

add_subdirectory (simplify)
add_subdirectory (util)

add_executable (generate_parser_action_table table.cpp)
target_link_libraries (generate_parser_action_table we)

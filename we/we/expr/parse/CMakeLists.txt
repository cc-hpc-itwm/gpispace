# -*- mode: cmake; -*-
set (hdrs
  action.hpp
  node.hpp
  parser.hpp
  )

set (srcs
  parser.cpp
  node.cpp
  action.cpp
)

add_library (we_expr_parse ${srcs})
add_we_library (we_expr_parse)

install(FILES ${hdrs}
  DESTINATION include/we/expr/parse
  COMPONENT headers)

add_subdirectory (simplify)
add_subdirectory (util)

add_executable (generate_parser_action_table table.cpp)
target_link_libraries (generate_parser_action_table
                       we_expr_parse
                       we_expr_token
                      )

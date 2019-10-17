fhg_add_test (NAME we_expr_parse_simplify
  SOURCES simplify.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_expr_parse_node_collect_key_roots
  SOURCES node_collect_key_roots.cpp
  USE_BOOST
  LIBRARIES pnet we-dev-testing
)

gspc_add_library (NAME gspc_we_testing
  TYPE STATIC
  SOURCES "testing/all_tokens.cpp"
  LIBRARIES gspc_we
)

gspc_add_test (NAME we_expr_parse_node_collect_key_roots
  SOURCES node_collect_key_roots.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_we_testing gspc_testing
)

gspc_add_test (NAME we_expr_parse_parser_collect_key_roots
  SOURCES parser_collect_key_roots.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_we_testing gspc_testing
)

gspc_add_library (NAME we_expr_type_testing
  TYPE STATIC
  SOURCES "type/testing/types.cpp"
  LIBRARIES gspc_we
)

gspc_add_test (NAME we_expr_parser
  SOURCES parser.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_util gspc_testing
)

gspc_add_test (NAME we_expr_parser_performance
  SOURCES parser.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  LIBRARIES gspc_we gspc_util gspc_testing
)

gspc_add_test (NAME we_expr_parser_type_check
  SOURCES parser.type_check.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_util gspc_testing
)

gspc_add_test (NAME we_expr_type_Context
  SOURCES "type/Context.cpp"
  USE_BOOST
  LIBRARIES gspc_we we_expr_type_testing gspc_util gspc_util gspc_testing
)
gspc_add_test (NAME we_expr_type_AssignResult
  SOURCES "type/AssignResult.cpp"
  USE_BOOST
  LIBRARIES gspc_we we_expr_type_testing gspc_util gspc_util gspc_testing
)

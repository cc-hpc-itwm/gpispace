fhg_add_test (NAME we_expr_parser
  SOURCES parser.cpp
  USE_BOOST
  LIBRARIES pnet fhg-util
)

fhg_add_test (NAME we_expr_parser_performance
  SOURCES parser.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  LIBRARIES pnet fhg-util
)

fhg_add_test (NAME we_expr_parser_type_check
  SOURCES parser.type_check.cpp
  USE_BOOST
  LIBRARIES pnet fhg-util
)

fhg_add_test (NAME we_expr_type_Context
  SOURCES "type/Context.cpp"
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing fhg-util Util::Generic
)
fhg_add_test (NAME we_expr_type_AssignResult
  SOURCES "type/AssignResult.cpp"
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing fhg-util Util::Generic
)

# mirko.rahn@itwm.fraunhofer.de

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

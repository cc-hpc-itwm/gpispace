# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (parser.cpp
  BOOST_UNIT_TEST
  PROJECT we_expr
  LINK_LIBRARIES pnet fhg-util
)

fhg_add_test (parser.performance.cpp
  BOOST_UNIT_TEST
  PERFORMANCE_TEST
  PROJECT we_expr
  LINK_LIBRARIES pnet fhg-util
)

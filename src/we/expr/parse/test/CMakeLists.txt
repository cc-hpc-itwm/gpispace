# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (simplify.cpp
  BOOST_UNIT_TEST
  PROJECT we_expr_parse
  LINK_LIBRARIES pnet
)

fhg_add_test (parser.cpp
  BOOST_UNIT_TEST
  PROJECT we_expr_parse
  LINK_LIBRARIES pnet
)

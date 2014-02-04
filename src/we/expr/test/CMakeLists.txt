# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (rename.cpp
  PROJECT we_expr
  LINK_LIBRARIES pnet
)

fhg_add_test (parser.cpp
  BOOST_UNIT_TEST
  PROJECT we_expr
  LINK_LIBRARIES pnet
)

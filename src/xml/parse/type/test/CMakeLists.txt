# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (link.cpp
  PROJECT xml_parse_type
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet fhg-util
)

fhg_add_test (place.cpp
  PROJECT xml_parse_type
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet fhg-util Boost::program_options
)

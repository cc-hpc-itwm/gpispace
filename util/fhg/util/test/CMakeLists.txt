# bernd.loerwald@itwm.fraunhofer.de

fhg_add_test (alphanum.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-util
)

fhg_add_test (test_xml_stream.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-util
)

fhg_add_test (test_remove_prefix.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-util
)

fhg_add_test (num.cpp
  PROJECT fhg_util_test
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-util
)

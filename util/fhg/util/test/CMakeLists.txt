# bernd.loerwald@itwm.fraunhofer.de

fhg_add_test (alphanum.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (xml_stream.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (remove_prefix.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (num.cpp
  BOOST_UNIT_TEST
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

fhg_add_test (split.cpp
  PROJECT fhg_util
  LINK_LIBRARIES fhg-util
)

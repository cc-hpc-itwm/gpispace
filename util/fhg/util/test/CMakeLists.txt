# bernd.loerwald@itwm.fraunhofer.de

fhg_add_test (alphanum.cpp
  LINK_LIBRARIES fhg-util
)

fhg_add_test (test_xml_stream.cpp
  LINK_LIBRARIES fhg-util
  DESCRIPTION "Tests the xml stream implementation"
)

fhg_add_test (test_remove_prefix.cpp
  LINK_LIBRARIES fhg-util
  DESCRIPTION "Tests the remove_prefix implementation"
)

fhg_add_test (num.cpp
  PROJECT fhg_util_test
  LINK_LIBRARIES fhg-util
)

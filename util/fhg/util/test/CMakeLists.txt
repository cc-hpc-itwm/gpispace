# bernd.loerwald@itwm.fraunhofer.de

fhg_add_test (test_xml_stream.cpp
  LINK_LIBRARIES fhg-util
  DESCRIPTION "Tests the xml stream implementation"
)

fhg_add_test (test_either.cpp
  LINK_LIBRARIES fhg-util
  DESCRIPTION "Tests the either implementation"
)

fhg_add_test (test_remove_prefix.cpp
  LINK_LIBRARIES fhg-util
  DESCRIPTION "Tests the remove_prefix implementation"
)

gspc_add_test (NAME xml_parse_type_place
  SOURCES place.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse gspc_util Boost::program_options gspc_testing
)

#bernd.loerwald@itwm.fraunhofer.de

set (XML_PARSE_TEST_PNETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/pnets")
set (XML_PARSE_TEST_XPNETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xpnets")

configure_file (parser_fixture.hpp.in parser_fixture.hpp @ONLY)

fhg_add_test (parse_duplicate_connect.cpp
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LINK_LIBRARIES pnet Boost::filesystem Boost::program_options
  DESCRIPTION "pnetc shall throw if there are mutiple connections with the same port, place and direction"
)

fhg_add_test (parse_compile_and_compare.cpp
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LINK_LIBRARIES pnet Boost::filesystem Boost::program_options
  DESCRIPTION "pnetc shall produce the same output as given in the reference files"
)

fhg_add_test (util_position.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet Boost::filesystem
  DESCRIPTION "test xml_util_position"
)

fhg_add_test (memory_buffer.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
)

fhg_add_test (memory_transfer.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
)

fhg_add_test (parse_memory_buffer.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
                 Boost::program_options
)

fhg_add_test (parse_memory_transfer.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
                 Boost::program_options
)

fhg_add_test (conditions_and_expressions_with_cdata.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
                 Boost::program_options
)

add_subdirectory (bytearray)
add_subdirectory (diagnostics)

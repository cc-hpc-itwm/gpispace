#bernd.loerwald@itwm.fraunhofer.de

if (${Boost_VERSION} EQUAL 104700)
  set (XML_PARSE_TEST_PNETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/pnets/1_47")
else()
  set (XML_PARSE_TEST_PNETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/pnets")
endif()

set (XML_PARSE_TEST_XPNETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xpnets")

configure_file (parser_fixture.hpp.in parser_fixture.hpp @ONLY)

include_directories (${CMAKE_CURRENT_BINARY_DIR})

fhg_add_test (parse_duplicate_connect.cpp
  LINK_LIBRARIES xml we
  DESCRIPTION "pnetc shall throw if there are mutiple connections with the same port, place and direction"
)

fhg_add_test (parse_compile_and_compare.cpp
  LINK_LIBRARIES xml we
  DESCRIPTION "pnetc shall produce the same output as given in the reference files"
)

fhg_add_test (util_position.cpp
  LINK_LIBRARIES xml
  DESCRIPTION "test xml_util_position"
)

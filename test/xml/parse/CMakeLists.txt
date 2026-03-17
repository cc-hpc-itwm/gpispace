configure_file (parser_fixture.hpp.in parser_fixture.hpp @ONLY)

gspc_add_test (NAME xml_parse_duplicate_connect
  SOURCES duplicate_connect.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES gspc_xml_parse
            gspc_testing
  DESCRIPTION "pnetc shall throw if there are mutiple connections with the same port, place and direction"
)

gspc_add_test (NAME xml_parse_memory_buffer
  SOURCES memory_buffer.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
)

gspc_add_test (NAME xml_parse_memory_transfer
  SOURCES memory_transfer.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
)

gspc_add_test (NAME xml_parse_invalid_preference_list
  SOURCES invalid_preference_list.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
)

gspc_add_test (NAME xml_parse_valid_preference_list
  SOURCES valid_preference_list.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
)

gspc_add_test (NAME xml_parse_invalid_eureka
  SOURCES invalid_eureka.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES gspc_xml_parse
            gspc_testing
  DESCRIPTION "pnetc shall throw for mutiple eureka connections with the same port and no output port for eureka"
)

gspc_add_test (NAME xml_parse_output_not_connected
  SOURCES output_not_connected.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
  DESCRIPTION "pnetc shall throw if an output port is not connected"
)

gspc_add_test (NAME xml_parse_conflicting_port_types
  SOURCES conflicting_port_types.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
  DESCRIPTION "warn when in/out port share a name but have different types"
)

add_subdirectory (type)

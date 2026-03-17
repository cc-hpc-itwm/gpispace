set (XML_PARSE_TEST_XPNETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xpnets")

gspc_add_test (NAME xml_util_position
  SOURCES util_position.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
  DESCRIPTION "test xml_util_position"
)

gspc_add_test (NAME xml_memory_buffer
  SOURCES memory_buffer.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
)

gspc_add_test (NAME xml_memory_transfer
  SOURCES memory_transfer.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
)

gspc_add_test (NAME xml_conditions_and_expressions_with_cdata
  SOURCES conditions_and_expressions_with_cdata.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/virtual_place_with_token_can_be_synthesized.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_with_token_can_be_synthesized.cpp
  @ONLY
)

gspc_add_test (NAME xml_parse_virtual_place_with_token_can_be_synthesized
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_with_token_can_be_synthesized.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_util
            gspc_testing
)

gspc_add_test (NAME xml_module_pass_context
  SOURCES module_pass_context.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
)

gspc_add_test (NAME xml_module_function_signature_variants
  SOURCES module_function_signature_variants.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
  DESCRIPTION "test all valid forms of the module function attribute grammar"
)

gspc_add_test (NAME xml_token_value_literals
  SOURCES token_value_literals.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse
            gspc_testing
  DESCRIPTION "test inline token value parsing for all literal types"
)

add_subdirectory (bytearray)
add_subdirectory (diagnostics)
add_subdirectory (parse)
add_subdirectory (template_with_module_call)

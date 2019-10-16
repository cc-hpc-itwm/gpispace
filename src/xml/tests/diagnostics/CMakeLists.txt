configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/function_description_with_unknown_port.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/function_description_with_unknown_port.cpp
  @ONLY
)

fhg_add_test (NAME xml_parse_function_description_with_unknown_port
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/function_description_with_unknown_port.cpp
  USE_BOOST
  LIBRARIES pnet
            Boost::filesystem
)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/virtual_place_not_tunneled.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_not_tunneled.cpp
  @ONLY
)

fhg_add_test (NAME xml_parse_virtual_place_not_tunneled
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_not_tunneled.cpp
  USE_BOOST
  LIBRARIES pnet
            Boost::filesystem
)

fhg_add_test (NAME xml_parse_diagnostics
  SOURCES diagnostics.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/.."
  LIBRARIES pnet Boost::filesystem
  DESCRIPTION "check parser diagnostics"
)

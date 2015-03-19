configure_file (
 ${CMAKE_CURRENT_SOURCE_DIR}/function_description_with_unknown_port.cpp.in
 ${CMAKE_CURRENT_BINARY_DIR}/function_description_with_unknown_port.cpp
 @ONLY
)

fhg_add_test (
 ${CMAKE_CURRENT_BINARY_DIR}/function_description_with_unknown_port.cpp
 BOOST_UNIT_TEST
 LINK_LIBRARIES pnet
                Boost::filesystem
                Boost::program_options
)

configure_file (
 ${CMAKE_CURRENT_SOURCE_DIR}/virtual_place_not_tunneled.cpp.in
 ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_not_tunneled.cpp
 @ONLY
)

fhg_add_test (
 ${CMAKE_CURRENT_BINARY_DIR}/virtual_place_not_tunneled.cpp
 BOOST_UNIT_TEST
 LINK_LIBRARIES pnet
                Boost::filesystem
                Boost::program_options
)

fhg_add_test (diagnostics.cpp
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/.."
  LINK_LIBRARIES pnet Boost::filesystem Boost::program_options
  DESCRIPTION "check parser diagnostics"
)

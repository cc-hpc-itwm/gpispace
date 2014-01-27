configure_file (
 ${CMAKE_CURRENT_SOURCE_DIR}/function_description_with_unknown_port.cpp.in
 ${CMAKE_CURRENT_BINARY_DIR}/function_description_with_unknown_port.cpp
 @ONLY
)

fhg_add_test (
 ${CMAKE_CURRENT_BINARY_DIR}/function_description_with_unknown_port.cpp
 BOOST_UNIT_TEST
 LINK_LIBRARIES pnet
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_PROGRAM_OPTIONS_LIBRARY}
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
                ${Boost_FILESYSTEM_LIBRARY}
                ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

fhg_add_test (diagnostics.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY}
  DESCRIPTION "check parser diagnostics"
)

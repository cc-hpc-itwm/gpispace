# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (layer.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (layer.ping_pong.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet Boost::program_options test-utilities
  ARGS --source-directory ${CMAKE_SOURCE_DIR}
)

fhg_add_test (layer.performance.cpp
  BOOST_UNIT_TEST
  PERFORMANCE_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (require_type.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (signature_of.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (exception.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (context.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (expression_based_cross_product.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
)

fhg_add_test (net.cpp
  BOOST_UNIT_TEST
  PROJECT we
  LINK_LIBRARIES pnet
                 Util::Generic
)

add_subdirectory (transition_cannot_fire_when_input_port_is_not_connected)
add_subdirectory (input_is_copied_to_output_in_module_calls)

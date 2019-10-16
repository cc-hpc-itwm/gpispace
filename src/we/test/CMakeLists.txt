# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (NAME we_layer
  SOURCES layer.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_layer.ping_pong
  SOURCES layer.ping_pong.cpp
  USE_BOOST
  LIBRARIES pnet Boost::program_options test-utilities
  ARGS --source-directory ${CMAKE_SOURCE_DIR}
)

fhg_add_test (NAME we_layer.performance
  SOURCES layer.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  LIBRARIES pnet
)

fhg_add_test (NAME we_require_type
  SOURCES require_type.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_signature_of
  SOURCES signature_of.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_exception
  SOURCES exception.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_context
  SOURCES context.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_expression_based_cross_product
  SOURCES expression_based_cross_product.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_tp_many_typecheck
  SOURCES tp_many_typecheck.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_net
  SOURCES net.cpp
  USE_BOOST
  LIBRARIES pnet
            Util::Generic
)

fhg_add_test (NAME we_net_performance
  SOURCES net.performance.cpp
  USE_BOOST
  LIBRARIES pnet
            Util::Generic
  PERFORMANCE_TEST
)

add_subdirectory (transition_cannot_fire_when_input_port_is_not_connected)
add_subdirectory (input_is_copied_to_output_in_module_calls)
add_subdirectory (put_many)

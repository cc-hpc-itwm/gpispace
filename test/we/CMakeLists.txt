gspc_add_library (NAME test_equality
  TYPE STATIC
  NAMESPACE we
  SOURCES "operator_equal.cpp"
  LIBRARIES gspc_we
)
gspc_add_library (NAME test_net_common
  TYPE STATIC
  NAMESPACE we
  SOURCES "net.common.cpp"
  LIBRARIES gspc_we
            gspc_testing
)

gspc_add_test (NAME we_layer
  SOURCES layer.cpp
  USE_BOOST
  LIBRARIES gspc_we we::test_equality gspc_testing
)

gspc_add_test (NAME we_layer.ping_pong
  SOURCES layer.ping_pong.cpp
  USE_BOOST
  LIBRARIES gspc_xml_parse Boost::program_options gspc_testing
  ARGS --source-directory ${PROJECT_SOURCE_DIR}
)

gspc_add_test (NAME we_layer.performance
  SOURCES layer.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  LIBRARIES gspc_we gspc_testing
)

gspc_add_test (NAME we_require_type
  SOURCES require_type.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_testing
)

gspc_add_test (NAME we_signature_of
  SOURCES signature_of.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_testing
)

gspc_add_test (NAME we_exception
  SOURCES exception.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_testing
)

gspc_add_test (NAME we_context
  SOURCES context.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_testing
)

gspc_add_test (NAME we_expression_based_cross_product
  SOURCES expression_based_cross_product.cpp
  USE_BOOST
  LIBRARIES gspc_we we::test_net_common
)

gspc_add_test (NAME we_tp_many_typecheck
  SOURCES tp_many_typecheck.cpp
  USE_BOOST
  LIBRARIES gspc_we we::test_net_common
)

gspc_add_test (NAME we_tp_many_duplicate_connections
  SOURCES tp_many_duplicate_connections.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_testing
)

gspc_add_test (NAME we_net
  SOURCES net.cpp
  USE_BOOST
  LIBRARIES gspc_we
            we::test_net_common
            gspc_util
)

gspc_add_test (NAME we_net_performance
  SOURCES net.performance.cpp
  USE_BOOST
  LIBRARIES gspc_we
            we::test_net_common
            gspc_util
  PERFORMANCE_TEST
)

gspc_add_test (NAME we_eureka_response
  SOURCES eureka_response.cpp
  USE_BOOST
  LIBRARIES gspc_we gspc_testing
)

if (GSPC_WITH_IML)
  add_subdirectory (buffer_alignment)
endif()
add_subdirectory (eureka)
add_subdirectory (input_is_copied_to_output_in_module_calls)
add_subdirectory (put_many)
add_subdirectory (transition_cannot_fire_when_input_port_is_not_connected)
add_subdirectory (plugins)

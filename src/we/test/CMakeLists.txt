if (BUILD_TESTING)
  gspc_add_library (NAME test_equality
    TYPE STATIC
    NAMESPACE we
    SOURCES "operator_equal.cpp"
    LIBRARIES pnet
  )
  gspc_add_library (NAME test_net_common
    TYPE STATIC
    NAMESPACE we
    SOURCES "net.common.cpp"
    LIBRARIES pnet
  )
endif()

fhg_add_test (NAME we_layer
  SOURCES layer.cpp
  USE_BOOST
  LIBRARIES pnet we::test_equality
)

fhg_add_test (NAME we_layer.ping_pong
  SOURCES layer.ping_pong.cpp
  USE_BOOST
  LIBRARIES pnet Boost::program_options test-utilities
  ARGS --source-directory ${PROJECT_SOURCE_DIR}
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
  LIBRARIES pnet we::test_net_common
)

fhg_add_test (NAME we_tp_many_typecheck
  SOURCES tp_many_typecheck.cpp
  USE_BOOST
  LIBRARIES pnet we::test_net_common
)

fhg_add_test (NAME we_tp_many_duplicate_connections
  SOURCES tp_many_duplicate_connections.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_net
  SOURCES net.cpp
  USE_BOOST
  LIBRARIES pnet
            we::test_net_common
            Util::Generic
)

fhg_add_test (NAME we_net_performance
  SOURCES net.performance.cpp
  USE_BOOST
  LIBRARIES pnet
            we::test_net_common
            Util::Generic
  PERFORMANCE_TEST
)

fhg_add_test (NAME we_eureka_response
  SOURCES eureka_response.cpp
  USE_BOOST
  LIBRARIES pnet
)

if (GSPC_WITH_IML)
  add_subdirectory (buffer_alignment)
endif()
add_subdirectory (eureka)
add_subdirectory (input_is_copied_to_output_in_module_calls)
add_subdirectory (put_many)
add_subdirectory (transition_cannot_fire_when_input_port_is_not_connected)
add_subdirectory (plugins)

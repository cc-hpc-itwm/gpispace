gspc_add_library (NAME gspc_logging_test
  TYPE STATIC
  SOURCES "message.cpp"
  LIBRARIES
    PUBLIC gspc_logging
    PUBLIC gspc_testing
)

add_unit_test (NAME logging_endpoint
  SOURCES "endpoint.cpp"
  LIBRARIES gspc_util
            gspc_logging_test
            gspc_testing
  USE_BOOST
)

add_unit_test (NAME logging_file_sink
  SOURCES "file_sink.cpp"
  LIBRARIES gspc_util
            gspc_logging_test
            gspc_testing
  USE_BOOST
)

add_unit_test (NAME logging_ostream_redirect
  SOURCES "ostream_redirect.cpp"
  LIBRARIES gspc_util
            gspc_logging_test
  USE_BOOST
)

add_unit_test (NAME logging_socket_endpoint
  SOURCES "socket_endpoint.cpp"
  LIBRARIES gspc_util
            gspc_logging_test
            gspc_testing
  USE_BOOST
)

add_unit_test (NAME logging_stream_emitter
  SOURCES "stream_emitter.cpp"
  LIBRARIES gspc_util
            gspc_logging_test
            gspc_testing
  USE_BOOST
)

add_unit_test (NAME logging_stream_receiver
  SOURCES "stream_receiver.cpp"
  LIBRARIES gspc_util
            gspc_logging_test
            gspc_testing
  USE_BOOST
)

add_unit_test (NAME logging_tcp_endpoint
  SOURCES "tcp_endpoint.cpp"
  LIBRARIES gspc_util
            gspc_logging_test
            gspc_testing
  USE_BOOST
)

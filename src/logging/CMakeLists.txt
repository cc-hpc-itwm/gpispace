gspc_add_library (NAME gspc_logging
  TYPE STATIC
  SOURCES
    "demultiplexer.cpp"
    "endpoint.cpp"
    "file_sink.cpp"
    "message.cpp"
    "socket_endpoint.cpp"
    "stdout_sink.cpp"
    "stream_emitter.cpp"
    "stream_receiver.cpp"
    "scoped_log_backtrace_and_exit_for_critical_errors.cpp"
    "tcp_endpoint.cpp"
    "tcp_server_providing_add_emitters.cpp"
  INCLUDE_DIRECTORIES
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES
    PUBLIC gspc_rif_started_process_promise
    PUBLIC gspc_util
    PUBLIC gspc_rpc
    PUBLIC Boost::base
    PUBLIC Boost::program_options
)

install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/logging/endpoint.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/logging/endpoint.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/logging/socket_endpoint.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/logging/socket_endpoint.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/logging/tcp_endpoint.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/logging/tcp_endpoint.ipp"
  DESTINATION include/gspc/logging
)

gspc_add_executable (NAME gspc-logging-demultiplexer
  SOURCES "gspc-logging-demultiplexer.cpp"
  LIBRARIES gspc_logging
            gspc_util
            gspc_rif_started_process_promise
  CREATE_BUNDLE_INFO
  INSTALL INSTALL_DESTINATION "libexec/gspc"
)

gspc_add_executable (NAME gspc-logging-to-stdout
  SOURCES "gspc-logging-to-stdout.cpp"
  LIBRARIES gspc_logging
            gspc_util
  INSTALL INSTALL_DESTINATION "bin"
)

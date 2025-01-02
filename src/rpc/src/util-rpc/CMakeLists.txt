extended_add_library (NAME RPC
  NAMESPACE Util
  INCLUDE_DIRECTORIES
    PRIVATE "${PROJECT_SOURCE_DIR}/src"
    PUBLIC $<BUILD_INTERFACE: ${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE: ${CMAKE_INSTALL_INCLUDEDIR}>
  SOURCES
    "detail/async_task_termination_guard.cpp"
    "locked_with_info_file.cpp"
    "remote_socket_endpoint.cpp"
    "remote_tcp_endpoint.cpp"
    "service_dispatcher.cpp"
    "service_socket_provider.cpp"
    "service_tcp_provider.cpp"
  LIBRARIES
    PUBLIC
      Util::Generic
      Boost::boost
      Boost::coroutine
      Boost::filesystem
      Boost::iostreams
      Boost::serialization
      Boost::system
      Boost::thread
    PRIVATE fmt
  POSITION_INDEPENDENT
  INSTALL
)
target_compile_features (Util-RPC
  PUBLIC "${_project_cxx_std}"
)

# Boost.Asio's strands share queues for sake of performance. There by
# default are 193 queues and a "random" distribution from strand to
# queue. By defining this config flag, the randomness is removed at
# least but the reuse persists.
# Enabling this is debatable but feels like the more "natural"
# assumption one would have about strands: The operations of a strand
# only block between themselves, not with operations of some other
# random strand.
# Assuming no client has more than 193 operations / strands /
# coroutines / service_providers / remote_endpoints, which should be a
# sane assumption other than for performance tests, this helps users
# of RPC to be a bit less correct with their callbacks as strands are
# now reused with a chance of rolling 193 strands, rather than a
# chance of up to 1 depending on the *address of the strand object*.
target_compile_definitions (Util-RPC
  PUBLIC
    BOOST_ASIO_ENABLE_SEQUENTIAL_STRAND_ALLOCATION
)

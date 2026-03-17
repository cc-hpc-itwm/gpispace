gspc_add_library (NAME gspc_rpc
  TYPE SHARED
  INCLUDE_DIRECTORIES
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
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
      gspc_util
      Boost::boost
      Boost::coroutine
      Boost::iostreams
      Boost::serialization
      Boost::system
      Boost::thread
    PRIVATE fmt::fmt
  INSTALL
  CREATE_BUNDLE_INFO
)

# GCC does not honour the GSPC_EXPORT attribute on template class
# definitions when -fvisibility=hidden is active: explicit template
# instantiations still receive hidden visibility.  Override to
# default so that all symbols of the library are exported.
# Non-template classes are annotated with GSPC_EXPORT for
# documentation and forward-compatibility.
set_target_properties (gspc_rpc
  PROPERTIES
    C_VISIBILITY_PRESET default
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN false
)

# Asio's strands share queues for sake of performance. There by
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
target_compile_definitions (gspc_rpc
  PUBLIC
    BOOST_ASIO_ENABLE_SEQUENTIAL_STRAND_ALLOCATION
)

install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/common.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/function_description.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/future.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/future.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/locked_with_info_file.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/remote_endpoint.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/remote_function.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/remote_function.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/remote_socket_endpoint.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/remote_tcp_endpoint.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/service_dispatcher.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/service_handler.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/service_handler.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/service_socket_provider.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/service_tcp_provider.hpp"
  DESTINATION include/gspc/rpc
)
install (FILES
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/async_task_termination_guard.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/async_task_termination_guard.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/packet_header.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/remote_stream_endpoint.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/remote_stream_endpoint.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/service_stream_provider.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/service_stream_provider.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/socket_traits.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/unique_scoped_handler_insert.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/unique_scoped_handler_insert.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/vector_sink.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/vector_sink.ipp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/yield_context_with_hooks.hpp"
  "${PROJECT_SOURCE_DIR}/include/gspc/rpc/detail/yield_context_with_hooks.ipp"
  DESTINATION include/gspc/rpc/detail
)

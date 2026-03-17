#include <gspc/rpc/detail/service_stream_provider.ipp>
#include <gspc/rpc/service_socket_provider.hpp>



    namespace gspc::rpc::detail
    {
      template struct service_stream_provider
        <::boost::asio::local::stream_protocol, service_socket_provider_traits>;
      template struct service_stream_provider_with_deferred_start
        <::boost::asio::local::stream_protocol, service_socket_provider_traits>;
      template struct service_stream_provider_with_deferred_dispatcher
        <::boost::asio::local::stream_protocol, service_socket_provider_traits>;
    }

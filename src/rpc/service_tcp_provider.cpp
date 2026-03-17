#include <gspc/rpc/detail/service_stream_provider.ipp>
#include <gspc/rpc/service_tcp_provider.hpp>



    namespace gspc::rpc::detail
    {
      template struct service_stream_provider
        <::boost::asio::ip::tcp, service_tcp_provider_traits>;
      template struct service_stream_provider_with_deferred_start
        <::boost::asio::ip::tcp, service_tcp_provider_traits>;
      template struct service_stream_provider_with_deferred_dispatcher
        <::boost::asio::ip::tcp, service_tcp_provider_traits>;
    }

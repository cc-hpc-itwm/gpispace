#include <gspc/rpc/detail/remote_stream_endpoint.ipp>
#include <gspc/rpc/remote_socket_endpoint.hpp>



    namespace gspc::rpc::detail
    {
      template struct remote_stream_endpoint
        <::boost::asio::local::stream_protocol, remote_socket_endpoint_traits>;
    }

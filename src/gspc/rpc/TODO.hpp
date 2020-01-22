#pragma once

#include <logging/endpoint.hpp>

#include <rpc/function_description.hpp>
#include <rpc/remote_endpoint.hpp>
#include <rpc/remote_function.hpp>
#include <rpc/remote_socket_endpoint.hpp>
#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_socket_provider.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <memory>

//! \todo merge rpc/logging
namespace rpc
{
  using endpoint = fhg::logging::endpoint;
  using tcp_endpoint = fhg::logging::tcp_endpoint;
  using socket_endpoint = fhg::logging::socket_endpoint;
  using remote_endpoint = fhg::rpc::remote_endpoint;
  using remote_tcp_endpoint = fhg::rpc::remote_tcp_endpoint;
  using remote_socket_endpoint = fhg::rpc::remote_socket_endpoint;
  using service_dispatcher = fhg::rpc::service_dispatcher;
  using service_socket_provider = fhg::rpc::service_socket_provider;
  using service_tcp_provider = fhg::rpc::service_tcp_provider;

  template<typename Protocol>
    using service_handler = fhg::rpc::service_handler<Protocol>;
  template<typename Protocol>
    using sync_remote_function = fhg::rpc::sync_remote_function<Protocol>;

  std::unique_ptr<remote_endpoint> make_endpoint
    (boost::asio::io_service& io_service, endpoint);
}

namespace fhg
{
  namespace logging
  {
    bool operator== (endpoint const&, endpoint const&);
  }
}

#include <functional>
#include <string>

namespace std
{
  template<>
    struct hash<fhg::logging::endpoint>
  {
    std::size_t operator() (fhg::logging::endpoint const& ep) const
    {
      return std::hash<std::string>{} (ep.to_string());
    }
  };
}

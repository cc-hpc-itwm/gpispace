#pragma once

#include <logging/endpoint.hpp>
#include <logging/message.hpp>
#include <logging/protocol.hpp>

#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_socket_provider.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace fhg
{
  namespace logging
  {
    class stream_receiver
    {
    public:
      using callback_t = std::function<void (message const&)>;
      stream_receiver (callback_t);
      stream_receiver (endpoint, callback_t);
      stream_receiver (std::vector<endpoint>, callback_t);

      void add_emitters (boost::asio::yield_context, std::vector<endpoint>);
      void add_emitters_blocking (std::vector<endpoint>);

    private:
      callback_t _callback;

      rpc::service_dispatcher _service_dispatcher;
      util::scoped_boost_asio_io_service_with_threads _io_service;
      rpc::service_handler<protocol::receive> const _receive;
      rpc::service_tcp_provider const _service_tcp_provider;
      rpc::service_socket_provider const _service_socket_provider;
      endpoint const _local_endpoint;
    };
  }
}

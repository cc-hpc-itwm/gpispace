#pragma once

#include <logging/message.hpp>
#include <logging/protocol.hpp>
#include <logging/tcp_endpoint.hpp>
#include <logging/tcp_endpoint_serialization.hpp>

#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
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
    class tcp_receiver
    {
    public:
      using endpoint_t = tcp_endpoint;
      using callback_t = std::function<void (message const&)>;
      tcp_receiver (callback_t);
      tcp_receiver (endpoint_t, callback_t);
      tcp_receiver (std::vector<endpoint_t>, callback_t);

      void add_emitters (std::vector<endpoint_t>);

    private:
      callback_t _callback;

      rpc::service_dispatcher _service_dispatcher;
      util::scoped_boost_asio_io_service_with_threads _io_service;
      rpc::service_handler<protocol::receive> const _receive;
      rpc::service_tcp_provider const _service_provider;
      std::pair<std::string, unsigned short> const _local_endpoint;
    };
  }
}

#pragma once

#include <logging/message.hpp>
#include <logging/protocol.hpp>
#include <logging/socket_endpoint.hpp>
#include <logging/tcp_endpoint.hpp>

#include <rpc/remote_endpoint.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_socket_provider.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <list>
#include <memory>

namespace fhg
{
  namespace logging
  {
    class stream_emitter
    {
    public:
      stream_emitter();

      socket_endpoint local_socket_endpoint() const;
      tcp_endpoint local_tcp_endpoint() const;

      void emit (message const&);

    private:
      rpc::service_dispatcher _service_dispatcher;
      util::scoped_boost_asio_io_service_with_threads _io_service;

      std::list<std::unique_ptr<rpc::remote_endpoint>> _receivers;

      void register_socket_receiver (socket_endpoint const&);
      rpc::service_handler<protocol::register_socket_receiver> const
        _register_socket_receiver;
      void register_tcp_receiver (tcp_endpoint const&);
      rpc::service_handler<protocol::register_tcp_receiver> const
        _register_tcp_receiver;

      rpc::service_socket_provider const _service_socket_provider;
      rpc::service_tcp_provider const _service_tcp_provider;
    };
  }
}

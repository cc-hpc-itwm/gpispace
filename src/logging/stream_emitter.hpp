// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <logging/endpoint.hpp>
#include <logging/message.hpp>
#include <logging/protocol.hpp>

#include <util-rpc/remote_endpoint.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_socket_provider.hpp>
#include <util-rpc/service_tcp_provider.hpp>

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

      endpoint local_endpoint() const;

      void emit_message (message const&);
      void emit_message (message const&, ::boost::asio::yield_context);
      void emit (decltype (message::_content), decltype (message::_category));

    private:
      rpc::service_dispatcher _service_dispatcher;
      util::scoped_boost_asio_io_service_with_threads _io_service;

      std::list<std::unique_ptr<rpc::remote_endpoint>> _receivers;

      void register_receiver (::boost::asio::yield_context, endpoint const&);
      rpc::service_handler<protocol::register_receiver> const
        _register_receiver;

      rpc::service_socket_provider const _service_socket_provider;
      rpc::service_tcp_provider const _service_tcp_provider;
      endpoint const _local_endpoint;
    };
  }
}

// Copyright (C) 2018-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/logging/endpoint.hpp>
#include <gspc/logging/message.hpp>
#include <gspc/logging/protocol.hpp>

#include <gspc/rpc/remote_endpoint.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_socket_provider.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>

#include <list>
#include <memory>


  namespace gspc::logging
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
      gspc::rpc::service_dispatcher _service_dispatcher;
      gspc::util::scoped_boost_asio_io_service_with_threads _io_service;

      std::list<std::unique_ptr<gspc::rpc::remote_endpoint>> _receivers;

      void register_receiver (::boost::asio::yield_context, endpoint const&);
      gspc::rpc::service_handler<protocol::register_receiver> const
        _register_receiver;

      gspc::rpc::service_socket_provider const _service_socket_provider;
      gspc::rpc::service_tcp_provider const _service_tcp_provider;
      endpoint const _local_endpoint;
    };
  }

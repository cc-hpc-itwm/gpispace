// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <logging/endpoint.hpp>
#include <logging/message.hpp>
#include <logging/protocol.hpp>

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

      endpoint local_endpoint() const;

      void emit_message (message const&);
      void emit_message (message const&, boost::asio::yield_context);
      void emit (decltype (message::_content), decltype (message::_category));

    private:
      rpc::service_dispatcher _service_dispatcher;
      util::scoped_boost_asio_io_service_with_threads _io_service;

      std::list<std::unique_ptr<rpc::remote_endpoint>> _receivers;

      void register_receiver (boost::asio::yield_context, endpoint const&);
      rpc::service_handler<protocol::register_receiver> const
        _register_receiver;

      rpc::service_socket_provider const _service_socket_provider;
      rpc::service_tcp_provider const _service_tcp_provider;
      endpoint const _local_endpoint;
    };
  }
}

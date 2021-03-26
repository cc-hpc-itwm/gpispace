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

#include <rif/entry_point.hpp>
#include <rif/protocol.hpp>

#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/remote_function.hpp>

namespace fhg
{
  namespace rif
  {
    class client
    {
    public:
      client ( boost::asio::io_service& io_service
             , fhg::rif::entry_point const& entry_point
             )
        : _endpoint (io_service, entry_point.hostname, entry_point.port)
        , execute_and_get_startup_messages (_endpoint)
        , execute_and_get_startup_messages_and_wait (_endpoint)
        , kill (_endpoint)
        , start_agent (_endpoint)
        , start_worker (_endpoint)
        , start_logging_demultiplexer (_endpoint)
        , add_emitter_to_logging_demultiplexer (_endpoint)
      {}

    private:
      fhg::rpc::remote_tcp_endpoint _endpoint;

    public:
      rpc::remote_function<protocol::execute_and_get_startup_messages>
        execute_and_get_startup_messages;
      rpc::remote_function<protocol::execute_and_get_startup_messages_and_wait>
        execute_and_get_startup_messages_and_wait;
      rpc::remote_function<protocol::kill> kill;
      rpc::remote_function<protocol::start_agent> start_agent;
      rpc::remote_function<protocol::start_worker> start_worker;
      rpc::remote_function<protocol::start_logging_demultiplexer>
        start_logging_demultiplexer;
      rpc::remote_function<protocol::add_emitter_to_logging_demultiplexer>
        add_emitter_to_logging_demultiplexer;
    };
  }
}

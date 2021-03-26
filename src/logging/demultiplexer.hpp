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

#include <logging/stream_emitter.hpp>
#include <logging/stream_receiver.hpp>

#include <rif/started_process_promise.hpp>

#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_socket_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

namespace fhg
{
  namespace logging
  {
    struct demultiplexer
    {
      stream_emitter emitter;
      stream_receiver receiver;

      util::scoped_boost_asio_io_service_with_threads io_service = {1};
      rpc::service_dispatcher service_dispatcher;
      rpc::service_handler<protocol::receiver::add_emitters> add_emitters;
      rpc::service_socket_provider add_service_provider;

      demultiplexer (rif::started_process_promise& promise, int, char**);
    };
  }
}

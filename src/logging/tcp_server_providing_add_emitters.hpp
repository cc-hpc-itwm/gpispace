// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <logging/protocol.hpp>
#include <logging/stream_receiver.hpp>

#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

namespace fhg
{
  namespace logging
  {
    //! Helper for the common pattern of having a server that allows
    //! adding emitters from external sources, e.g. DRTS startup.
    class tcp_server_providing_add_emitters
    {
    public:
      tcp_server_providing_add_emitters (stream_receiver*, unsigned short port);

    private:
      rpc::service_dispatcher service_dispatcher;
      rpc::service_handler<protocol::receiver::add_emitters> add_emitters;
      util::scoped_boost_asio_io_service_with_threads io_service = {1};
      rpc::service_tcp_provider add_emitters_service_provider;
    };
  }
}

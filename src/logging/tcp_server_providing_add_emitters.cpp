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

#include <logging/tcp_server_providing_add_emitters.hpp>

#include <util-generic/this_bound_mem_fn.hpp>

namespace fhg
{
  namespace logging
  {
    tcp_server_providing_add_emitters::tcp_server_providing_add_emitters
        (stream_receiver* log_receiver, unsigned short port)
      : add_emitters
          ( service_dispatcher
          , util::bind_this (log_receiver, &stream_receiver::add_emitters)
          , fhg::rpc::yielding
          )
      , add_emitters_service_provider
          ( io_service
          , service_dispatcher
          , boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4(), port)
          )
    {}
  }
}

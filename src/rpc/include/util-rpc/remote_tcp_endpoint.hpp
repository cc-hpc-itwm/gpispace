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

#include <util-rpc/detail/remote_stream_endpoint.hpp>

#include <boost/asio/ip/tcp.hpp>

namespace fhg
{
  namespace rpc
  {
    struct remote_tcp_endpoint_traits
    {
      template<typename Socket>
        static void apply_socket_options (Socket& socket)
      {
        socket.set_option (::boost::asio::ip::tcp::no_delay (true));
      }
    };

    //! A remote endpoint communicating via TCP/IP. The endpoint is a
    //! host+port pair which can be specified as pair or separate.
    //! \see service_tcp_provider
    struct remote_tcp_endpoint
      : detail::remote_stream_endpoint
          <::boost::asio::ip::tcp, remote_tcp_endpoint_traits>
    {
      remote_tcp_endpoint
        ( ::boost::asio::io_service& io_service
        , std::string host
        , unsigned short port
        , util::serialization::exception::serialization_functions functions
        = util::serialization::exception::serialization_functions()
        );

      remote_tcp_endpoint
        ( ::boost::asio::io_service& io_service
        , std::pair<std::string, unsigned short> host_port
        , util::serialization::exception::serialization_functions functions
        = util::serialization::exception::serialization_functions()
        )
          : remote_tcp_endpoint ( io_service
                                , std::move (host_port.first)
                                , host_port.second
                                , std::move (functions)
                                )
      {}

      remote_tcp_endpoint
        ( ::boost::asio::io_service& io_service
        , ::boost::asio::yield_context yield
        , std::string host
        , unsigned short port
        , util::serialization::exception::serialization_functions functions
        = util::serialization::exception::serialization_functions()
        );

      remote_tcp_endpoint
        ( ::boost::asio::io_service& io_service
        , ::boost::asio::yield_context yield
        , std::pair<std::string, unsigned short> host_port
        , util::serialization::exception::serialization_functions functions
        = util::serialization::exception::serialization_functions()
        )
          : remote_tcp_endpoint ( io_service
                                , yield
                                , std::move (host_port.first)
                                , host_port.second
                                , std::move (functions)
                                )
      {}
    };

    namespace detail
    {
      extern template struct remote_stream_endpoint
        <::boost::asio::ip::tcp, remote_tcp_endpoint_traits>;
    }
  }
}

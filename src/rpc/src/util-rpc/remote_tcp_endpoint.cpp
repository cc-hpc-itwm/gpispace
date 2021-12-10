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

#include <util-rpc/detail/remote_stream_endpoint.ipp>
#include <util-rpc/remote_tcp_endpoint.hpp>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template struct remote_stream_endpoint
        <::boost::asio::ip::tcp, remote_tcp_endpoint_traits>;
    }

    remote_tcp_endpoint::remote_tcp_endpoint
        ( ::boost::asio::io_service& io_service
        , std::string host
        , unsigned short port
        , util::serialization::exception::serialization_functions functions
        )
      : detail::remote_stream_endpoint<::boost::asio::ip::tcp, remote_tcp_endpoint_traits>
          ( io_service
          , ::boost::asio::ip::tcp::resolver (io_service)
          . resolve
              ( { host
                , std::to_string (port)
                , ::boost::asio::ip::resolver_query_base::numeric_service
                }
              )
          , std::move (functions)
          )
    {}

    remote_tcp_endpoint::remote_tcp_endpoint
        ( ::boost::asio::io_service& io_service
        , ::boost::asio::yield_context yield
        , std::string host
        , unsigned short port
        , util::serialization::exception::serialization_functions functions
        )
      : detail::remote_stream_endpoint<::boost::asio::ip::tcp, remote_tcp_endpoint_traits>
          ( io_service
          , std::move (yield)
          , ::boost::asio::ip::tcp::resolver (io_service)
          . resolve
              ( { host
                , std::to_string (port)
                , ::boost::asio::ip::resolver_query_base::numeric_service
                }
              )
          , std::move (functions)
          )
    {}
  }
}

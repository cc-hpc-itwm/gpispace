// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

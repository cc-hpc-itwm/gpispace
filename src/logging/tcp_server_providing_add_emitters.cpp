// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
          , ::boost::asio::ip::tcp::endpoint (::boost::asio::ip::tcp::v4(), port)
          )
    {}
  }
}

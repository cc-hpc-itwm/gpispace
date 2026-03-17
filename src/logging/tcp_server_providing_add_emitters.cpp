// Copyright (C) 2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/tcp_server_providing_add_emitters.hpp>

#include <gspc/util/this_bound_mem_fn.hpp>


  namespace gspc::logging
  {
    tcp_server_providing_add_emitters::tcp_server_providing_add_emitters
        (stream_receiver* log_receiver, unsigned short port)
      : add_emitters
          ( service_dispatcher
          , gspc::util::bind_this (log_receiver, &stream_receiver::add_emitters)
          , gspc::rpc::yielding
          )
      , add_emitters_service_provider
          ( io_service
          , service_dispatcher
          , ::boost::asio::ip::tcp::endpoint (::boost::asio::ip::tcp::v4(), port)
          )
    {}
  }

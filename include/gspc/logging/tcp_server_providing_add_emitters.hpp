// Copyright (C) 2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/logging/protocol.hpp>
#include <gspc/logging/stream_receiver.hpp>

#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>


  namespace gspc::logging
  {
    //! Helper for the common pattern of having a server that allows
    //! adding emitters from external sources, e.g. DRTS startup.
    class tcp_server_providing_add_emitters
    {
    public:
      tcp_server_providing_add_emitters (stream_receiver*, unsigned short port);

    private:
      gspc::rpc::service_dispatcher service_dispatcher;
      gspc::rpc::service_handler<protocol::receiver::add_emitters> add_emitters;
      gspc::util::scoped_boost_asio_io_service_with_threads io_service = {1};
      gspc::rpc::service_tcp_provider add_emitters_service_provider;
    };
  }

// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

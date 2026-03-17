// Copyright (C) 2019,2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/logging/stream_emitter.hpp>
#include <gspc/logging/stream_receiver.hpp>

#include <gspc/rif/started_process_promise.hpp>

#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_socket_provider.hpp>

#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>


  namespace gspc::logging
  {
    struct demultiplexer
    {
      stream_emitter emitter;
      stream_receiver receiver;

      gspc::util::scoped_boost_asio_io_service_with_threads io_service = {1};
      gspc::rpc::service_dispatcher service_dispatcher;
      gspc::rpc::service_handler<protocol::receiver::add_emitters> add_emitters;
      gspc::rpc::service_socket_provider add_service_provider;

      demultiplexer (gspc::rif::started_process_promise& promise, int, char**);
    };
  }

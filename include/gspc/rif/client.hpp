// Copyright (C) 2015-2016,2018-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/rif/entry_point.hpp>
#include <gspc/rif/protocol.hpp>

#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>


  namespace gspc::rif
  {
    class client
    {
    public:
      client ( ::boost::asio::io_service& io_service
             , gspc::rif::entry_point const& entry_point
             )
        : _endpoint (io_service, entry_point.hostname, entry_point.port)
        , execute_and_get_startup_messages (_endpoint)
        , execute_and_get_startup_messages_and_wait (_endpoint)
        , kill (_endpoint)
        , start_agent (_endpoint)
        , start_worker (_endpoint)
        , start_logging_demultiplexer (_endpoint)
        , add_emitter_to_logging_demultiplexer (_endpoint)
        , start_vmem (_endpoint)
      {}

    private:
      gspc::rpc::remote_tcp_endpoint _endpoint;

    public:
      gspc::rpc::remote_function<protocol::execute_and_get_startup_messages>
        execute_and_get_startup_messages;
      gspc::rpc::remote_function<protocol::execute_and_get_startup_messages_and_wait>
        execute_and_get_startup_messages_and_wait;
      gspc::rpc::remote_function<protocol::kill> kill;
      gspc::rpc::remote_function<protocol::start_agent> start_agent;
      gspc::rpc::remote_function<protocol::start_worker> start_worker;
      gspc::rpc::remote_function<protocol::start_logging_demultiplexer>
        start_logging_demultiplexer;
      gspc::rpc::remote_function<protocol::add_emitter_to_logging_demultiplexer>
        add_emitter_to_logging_demultiplexer;
      gspc::rpc::remote_function<protocol::start_vmem>
        start_vmem;
    };
  }

// Copyright (C) 2019-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/demultiplexer.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/this_bound_mem_fn.hpp>


  namespace gspc::logging
  {
    demultiplexer::demultiplexer
        (gspc::rif::started_process_promise& promise, int, char**)
      : receiver ( [&] (::boost::asio::yield_context yield, message const& m)
                   {
                     return emitter.emit_message (m, std::move (yield));
                   }
                 )
      , add_emitters
          ( service_dispatcher
          , gspc::util::bind_this (&receiver, &stream_receiver::add_emitters)
          , gspc::rpc::yielding
          )
      , add_service_provider (io_service, service_dispatcher)
    {
      promise.set_result
        ( emitter.local_endpoint().to_string()
        , socket_endpoint (add_service_provider.local_endpoint()).to_string()
        );
    }
  }

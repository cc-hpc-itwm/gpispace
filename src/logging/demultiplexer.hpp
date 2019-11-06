#pragma once

#include <logging/stream_emitter.hpp>
#include <logging/stream_receiver.hpp>

#include <rif/started_process_promise.hpp>

#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_socket_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

namespace fhg
{
  namespace logging
  {
    struct demultiplexer
    {
      stream_emitter emitter;
      stream_receiver receiver;

      util::scoped_boost_asio_io_service_with_threads io_service = {1};
      rpc::service_dispatcher service_dispatcher;
      rpc::service_handler<protocol::receiver::add_emitters> add_emitters;
      rpc::service_socket_provider add_service_provider;

      demultiplexer (rif::started_process_promise& promise, int, char**);
    };
  }
}

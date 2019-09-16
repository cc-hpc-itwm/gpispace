#include <logging/stream_emitter.hpp>

#include <rpc/remote_function.hpp>
#include <rpc/remote_socket_endpoint.hpp>
#include <rpc/remote_tcp_endpoint.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/this_bound_mem_fn.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <future>
#include <vector>

namespace fhg
{
  namespace logging
  {
    stream_emitter::stream_emitter()
      : _io_service (2)
      , _register_socket_receiver
          ( _service_dispatcher
          , util::bind_this (this, &stream_emitter::register_socket_receiver)
          )
      , _register_tcp_receiver
          ( _service_dispatcher
          , util::bind_this (this, &stream_emitter::register_tcp_receiver)
          )
      , _service_socket_provider (_io_service, _service_dispatcher)
      , _service_tcp_provider (_io_service, _service_dispatcher)
    {}

    socket_endpoint stream_emitter::local_socket_endpoint() const
    {
      return _service_socket_provider.local_endpoint();
    }
    tcp_endpoint stream_emitter::local_tcp_endpoint() const
    {
      return util::connectable_to_address_string
        (_service_tcp_provider.local_endpoint());
    }

    void stream_emitter::emit_message (message const& forwarded_message)
    {
      std::vector<std::future<void>> receiver_results;

      for (auto const& receiver : _receivers)
      {
        using fun = rpc::remote_function<protocol::receive>;
        receiver_results.emplace_back (fun {*receiver} (forwarded_message));
      }

      try
      {
        util::wait_and_collect_exceptions (receiver_results);
      }
      catch (...)
      {
        //! \todo Ignore for now. Report somewhere? Remove receiver
        //! from list to avoid endless errors?
      }
    }

    void stream_emitter::register_socket_receiver
      (socket_endpoint const& endpoint)
    {
      _receivers.emplace_back
        ( util::cxx14::make_unique<rpc::remote_socket_endpoint>
            (_io_service, endpoint.socket)
        );
    }
    void stream_emitter::register_tcp_receiver (tcp_endpoint const& endpoint)
    {
      _receivers.emplace_back
        ( util::cxx14::make_unique<rpc::remote_tcp_endpoint>
            (_io_service, endpoint)
        );
    }
  }
}

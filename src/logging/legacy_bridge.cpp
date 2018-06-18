#include <logging/legacy_bridge.hpp>

#include <rpc/remote_function.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/this_bound_mem_fn.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <future>
#include <vector>

namespace fhg
{
  namespace logging
  {
    legacy_bridge::legacy_bridge (unsigned short legacy_port)
      : legacy::receiver (legacy_port)
      , _io_service (2)
      , _register_sink ( _service_dispatcher
                       , [&] (tcp_endpoint const& endpoint)
                         {
                           _sinks.emplace_back (_io_service, endpoint);
                         }
                       )
      , _service_provider (_io_service, _service_dispatcher)
    {}

    tcp_endpoint legacy_bridge::local_endpoint() const
    {
      return util::connectable_to_address_string
        (_service_provider.local_endpoint());
    }

    void legacy_bridge::on_legacy (legacy::event const& event)
    {
      std::vector<std::future<void>> sink_results;

      message const forwarded_message (message::from_legacy (event));
      for (auto& sink : _sinks)
      {
        using fun = rpc::remote_function<protocol::sink>;
        sink_results.emplace_back (fun {sink} (forwarded_message));
      }

      try
      {
        util::wait_and_collect_exceptions (sink_results);
      }
      catch (...)
      {
        //! \todo Ignore for now. Report somewhere? Throw? Remove sink
        //! from list to avoid endless errors?
      }
    }

    legacy_bridge::receiver::receiver
        (tcp_endpoint const& emitter, callback_t callback)
      : _callback (std::move (callback))
      , _io_service (1)
      , _sink ( _service_dispatcher
              , [this] (message const& message) { return _callback (message); }
              )
      , _service_provider (_io_service, _service_dispatcher)
    {
      rpc::remote_tcp_endpoint client (_io_service, emitter);
      rpc::sync_remote_function<protocol::register_sink> {client}
        ( util::connectable_to_address_string
            (_service_provider.local_endpoint())
        );
    }
  }
}

#include <logging/tcp_receiver.hpp>

#include <rpc/remote_function.hpp>
#include <rpc/remote_tcp_endpoint.hpp>

#include <util-generic/connectable_to_address_string.hpp>

namespace fhg
{
  namespace logging
  {
    tcp_receiver::tcp_receiver
        (endpoint_t const& emitter, callback_t callback)
      : _callback (std::move (callback))
      , _io_service (1)
      , _receive
          ( _service_dispatcher
          , [this] (message const& message) { return _callback (message); }
          )
      , _service_provider (_io_service, _service_dispatcher)
    {
      rpc::remote_tcp_endpoint client (_io_service, emitter);
      rpc::sync_remote_function<protocol::register_tcp_receiver> {client}
        ( util::connectable_to_address_string
            (_service_provider.local_endpoint())
        );
    }
  }
}

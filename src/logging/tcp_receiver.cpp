#include <logging/tcp_receiver.hpp>

#include <rpc/remote_function.hpp>
#include <rpc/remote_tcp_endpoint.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <future>

namespace fhg
{
  namespace logging
  {
    tcp_receiver::tcp_receiver (callback_t callback)
      : tcp_receiver (std::list<endpoint_t>(), std::move (callback))
    {}

    tcp_receiver::tcp_receiver (endpoint_t emitter, callback_t callback)
      : tcp_receiver ( std::list<endpoint_t> {std::move (emitter)}
                     , std::move (callback)
                     )
    {}

    tcp_receiver::tcp_receiver ( std::list<endpoint_t> emitters
                               , callback_t callback
                               )
      : _callback (std::move (callback))
      , _io_service (1)
      , _receive
          ( _service_dispatcher
          , [this] (message const& message) { return _callback (message); }
          )
      , _service_provider (_io_service, _service_dispatcher)
      , _local_endpoint ( util::connectable_to_address_string
                            (_service_provider.local_endpoint())
                        )
    {
      add_emitters (std::move (emitters));
    }

    void tcp_receiver::add_emitters (std::list<endpoint_t> emitters)
    {
      std::list<rpc::remote_tcp_endpoint> endpoints;
      std::vector<std::future<void>> futures;
      for (auto& emitter : emitters)
      {
        using function = rpc::remote_function<protocol::register_tcp_receiver>;
        endpoints.emplace_back (_io_service, std::move (emitter));
        futures.emplace_back (function {endpoints.back()} (_local_endpoint));
      }

      util::wait_and_collect_exceptions (futures);
    }
  }
}

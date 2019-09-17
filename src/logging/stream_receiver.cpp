#include <logging/stream_receiver.hpp>

#include <rpc/remote_function.hpp>
#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/remote_socket_endpoint.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/functor_visitor.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <future>
#include <list>

namespace fhg
{
  namespace logging
  {
    stream_receiver::stream_receiver (callback_t callback)
      : stream_receiver (std::vector<endpoint>(), std::move (callback))
    {}

    stream_receiver::stream_receiver (endpoint emitter, callback_t callback)
      : stream_receiver ( std::vector<endpoint> {std::move (emitter)}
                        , std::move (callback)
                        )
    {}

    stream_receiver::stream_receiver ( std::vector<endpoint> emitters
                                     , callback_t callback
                                     )
      : _callback (std::move (callback))
      , _io_service (1)
      , _receive
          ( _service_dispatcher
          , [this] (message const& message) { return _callback (message); }
          )
      , _service_tcp_provider (_io_service, _service_dispatcher)
      , _service_socket_provider (_io_service, _service_dispatcher)
      , _local_endpoint ( util::connectable_to_address_string
                            (_service_tcp_provider.local_endpoint())
                        , _service_socket_provider.local_endpoint()
                        )
    {
      add_emitters (std::move (emitters));
    }

    void stream_receiver::add_emitters (std::vector<endpoint> emitters)
    {
      std::list<std::unique_ptr<rpc::remote_endpoint>> endpoints;
      std::vector<std::future<void>> futures;

      for (auto& emitter : emitters)
      {
        util::visit<void>
          ( emitter.best (_local_endpoint.as_socket->host)
          , [&] (socket_endpoint const& as_socket)
            {
              endpoints.emplace_back
                ( util::cxx14::make_unique<rpc::remote_socket_endpoint>
                    (_io_service, as_socket.socket)
                );
            }
          , [&] (tcp_endpoint const& as_tcp)
            {
              endpoints.emplace_back
                ( util::cxx14::make_unique<rpc::remote_tcp_endpoint>
                    (_io_service, as_tcp)
                );
            }
          );

        using function = rpc::remote_function<protocol::register_receiver>;
        futures.emplace_back (function {*endpoints.back()} (_local_endpoint));
      }

      util::wait_and_collect_exceptions (futures);
    }
  }
}

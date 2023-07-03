// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <logging/stream_receiver.hpp>

#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_socket_endpoint.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>

#include <util-generic/connectable_to_address_string.hpp>
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

    stream_receiver::stream_receiver (yielding_callback_t callback)
      : _io_service (1)
      , _receive (_service_dispatcher, std::move (callback), rpc::yielding)
      , _service_tcp_provider (_io_service, _service_dispatcher)
      , _service_socket_provider (_io_service, _service_dispatcher)
      , _local_endpoint ( util::connectable_to_address_string
                            (_service_tcp_provider.local_endpoint())
                        , _service_socket_provider.local_endpoint()
                        )
    {}

    stream_receiver::stream_receiver (endpoint emitter, callback_t callback)
      : stream_receiver ( std::vector<endpoint> {std::move (emitter)}
                        , std::move (callback)
                        )
    {}

    stream_receiver::stream_receiver ( std::vector<endpoint> emitters
                                     , callback_t callback
                                     )
      : _io_service (1)
      , _receive (_service_dispatcher, std::move (callback), rpc::not_yielding)
      , _service_tcp_provider (_io_service, _service_dispatcher)
      , _service_socket_provider (_io_service, _service_dispatcher)
      , _local_endpoint ( util::connectable_to_address_string
                            (_service_tcp_provider.local_endpoint())
                        , _service_socket_provider.local_endpoint()
                        )
    {
      add_emitters_blocking (std::move (emitters));
    }

    namespace
    {
      template<typename... MaybeYield>
        void add_emitters_impl ( std::vector<endpoint> emitters
                               , endpoint const& local_endpoint
                               , ::boost::asio::io_service& io_service
                               , MaybeYield... yield
                               )
      {
        std::list<std::unique_ptr<rpc::remote_endpoint>> endpoints;
        std::vector<std::future<void>> futures;

        for (auto& emitter : emitters)
        {
          util::visit<void>
            ( emitter.best (local_endpoint.as_socket->host)
            , [&] (socket_endpoint const& as_socket)
              {
                endpoints.emplace_back
                  ( std::make_unique<rpc::remote_socket_endpoint>
                      (io_service, yield..., as_socket.socket)
                  );
              }
            , [&] (tcp_endpoint const& as_tcp)
              {
                endpoints.emplace_back
                  ( std::make_unique<rpc::remote_tcp_endpoint>
                      (io_service, yield..., as_tcp)
                  );
              }
            );

          using function = rpc::remote_function<protocol::register_receiver>;
          futures.emplace_back (function {*endpoints.back()} (local_endpoint));
        }

        util::wait_and_collect_exceptions (futures);
      }
    }

    void stream_receiver::add_emitters
      (::boost::asio::yield_context yield, std::vector<endpoint> emitters)
    {
      return add_emitters_impl
        (std::move (emitters), _local_endpoint, _io_service, yield);
    }
    void stream_receiver::add_emitters_blocking (std::vector<endpoint> emitters)
    {
      return add_emitters_impl
        (std::move (emitters), _local_endpoint, _io_service);
    }
  }
}

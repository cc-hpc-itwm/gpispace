// Copyright (C) 2018-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/stream_emitter.hpp>

#include <gspc/rpc/future.hpp>
#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_socket_endpoint.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>

#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/functor_visitor.hpp>
#include <gspc/util/hostname.hpp>
#include <gspc/util/this_bound_mem_fn.hpp>
#include <gspc/util/wait_and_collect_exceptions.hpp>

#include <exception>
#include <future>
#include <utility>
#include <vector>


  namespace gspc::logging
  {
    stream_emitter::stream_emitter()
      : _io_service (2)
      , _register_receiver
          ( _service_dispatcher
          , gspc::util::bind_this (this, &stream_emitter::register_receiver)
          , gspc::rpc::yielding
          )
      , _service_socket_provider (_io_service, _service_dispatcher)
      , _service_tcp_provider (_io_service, _service_dispatcher)
      , _local_endpoint ( gspc::util::connectable_to_address_string
                            (_service_tcp_provider.local_endpoint())
                        , _service_socket_provider.local_endpoint()
                        )
    {}

    endpoint stream_emitter::local_endpoint() const
    {
      return _local_endpoint;
    }

    namespace
    {
      template < template<typename> class Future
               , typename Receivers
               , typename... Yield
               >
        void emit_message_impl ( Receivers& receivers
                               , message const& forwarded_message
                               , Yield... yield
                               )
      {
        std::vector<Future<void>> receiver_results;

        for (auto const& receiver : receivers)
        {
          using fun = gspc::rpc::remote_function<protocol::receive, Future>;
          receiver_results.emplace_back (fun {*receiver} (forwarded_message));
        }

        try
        {
          gspc::util::apply_for_each_and_collect_exceptions
            ( std::move (receiver_results)
            , [&] (Future<void>& future) { return future.get (yield...); }
            );
        }
        catch (...)
        {
          //! \todo Ignore for now. Report somewhere? Remove receiver
          //! from list to avoid endless errors?

          std::ignore = std::current_exception();
        }
      }
    }

    void stream_emitter::emit_message (message const& forwarded_message)
    {
      return emit_message_impl<std::future> (_receivers, forwarded_message);
    }
    void stream_emitter::emit_message
      (message const& forwarded_message, ::boost::asio::yield_context yield)
    {
      return emit_message_impl<gspc::rpc::future>
        (_receivers, forwarded_message, yield);
    }

    void stream_emitter::emit ( decltype (message::_content) content
                              , decltype (message::_category) category
                              )
    {
      return emit_message ({std::move (content), std::move (category)});
    }

    void stream_emitter::register_receiver
      (::boost::asio::yield_context yield, endpoint const& endpoint)
    {
      gspc::util::visit
        ( endpoint.best (_local_endpoint.as_socket->host)
        , [&] (socket_endpoint const& as_socket)
          {
            _receivers.emplace_back
              ( std::make_unique<gspc::rpc::remote_socket_endpoint>
                  (_io_service, yield, as_socket.socket)
              );
          }
        , [&] (tcp_endpoint const& as_tcp)
          {
            _receivers.emplace_back
              ( std::make_unique<gspc::rpc::remote_tcp_endpoint>
                  (_io_service, yield, as_tcp)
              );
          }
        );
    }
  }

// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <logging/stream_emitter.hpp>

#include <util-rpc/future.hpp>
#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_socket_endpoint.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/functor_visitor.hpp>
#include <util-generic/hostname.hpp>
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
      , _register_receiver
          ( _service_dispatcher
          , util::bind_this (this, &stream_emitter::register_receiver)
          , fhg::rpc::yielding
          )
      , _service_socket_provider (_io_service, _service_dispatcher)
      , _service_tcp_provider (_io_service, _service_dispatcher)
      , _local_endpoint ( util::connectable_to_address_string
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
          using fun = rpc::remote_function<protocol::receive, Future>;
          receiver_results.emplace_back (fun {*receiver} (forwarded_message));
        }

        try
        {
          util::apply_for_each_and_collect_exceptions
            ( std::move (receiver_results)
            , [&] (Future<void>& future) { return future.get (yield...); }
            );
        }
        catch (...)
        {
          //! \todo Ignore for now. Report somewhere? Remove receiver
          //! from list to avoid endless errors?
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
      return emit_message_impl<rpc::future>
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
      util::visit<void>
        ( endpoint.best (_local_endpoint.as_socket->host)
        , [&] (socket_endpoint const& as_socket)
          {
            _receivers.emplace_back
              ( std::make_unique<rpc::remote_socket_endpoint>
                  (_io_service, yield, as_socket.socket)
              );
          }
        , [&] (tcp_endpoint const& as_tcp)
          {
            _receivers.emplace_back
              ( std::make_unique<rpc::remote_tcp_endpoint>
                  (_io_service, yield, as_tcp)
              );
          }
        );
    }
  }
}

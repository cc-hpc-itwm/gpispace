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

#pragma once

#include <rpc/common.hpp>
#include <rpc/detail/async_task_termination_guard.hpp>
#include <rpc/detail/socket_traits.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>

#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <vector>

namespace fhg
{
  namespace rpc
  {
    struct service_dispatcher;
    namespace detail
    {
      //! A service stream provider listens on a stream of the given
      //! \a Protocol and forwards incoming function call requests to
      //! a given dispatcher. The setup of the infrastructure is split
      //! into two steps, to allow determining the endpoint to connect
      //! to before forking and actually handling requests.
      //!
      //! The standard usage is to
      //!
      //! - Set up a \c service_dispatcher with various
      //!   \c service_handler implementing the API.
      //!
      //! - Create a `boost::asio::io_service` and set up one or more
      //!   threads to handle IO. A convenience wrapper for this
      //!   exists as \c util::scoped_boost_asio_io_service_with_threads
      //!   or the `_and_deferred_startup` variant in case of a
      //!   daemonizing service.
      //!
      //! - Create a \c service_stream_provider or a
      //!   \c service_stream_provider_with_deferred_start in case of
      //!   a daemonizing service.
      //!
      //! - Communicate the `local_endpoint()` of the provider with
      //!   the outside world.
      //!
      //! - (Daemonizing service only) Fork and call `post_fork_*()`
      //!   on the `io_service` in the respective processes. Call
      //!   `start()` on the provider and set up IO threads by calling
      //!   `start_in_threads_and_current_thread()` on the `io_service`.
      //!
      //! \a Traits is used to apply options on incoming connections,
      //! calling \c Traits::apply_socket_options() on every socket.
      template<typename Protocol, typename Traits>
        struct service_stream_provider_with_deferred_start
      {
        static_assert
          ( is_socket_traits_t<Traits, Protocol>{}
          , "Traits shall have Traits::apply_socket_options (Socket&)"
          );

      public:
        //! Prepare the infrastructure to listen for function call
        //! requests on \a endpoint using \a io_service, and
        //! dispatching incoming requests using \a dispatcher. The
        //! provider is not handling calls until \c start() has been
        //! called.
        //!
        //! The \a endpoint may be default-constructed to let the
        //! provider automatically choose one. This prevents "already
        //! in use" errors, but requires to publish \c local_endpoint()
        //! after construction, i.e. needs a separate communication
        //! channel, e.g. a file or copying from `stdout`.
        //!
        //! \warn The given io_service _has_ to have at least one
        //! thread running. If there are no running threads, behaviour
        //! is undefined (and probably hangs during destructor).
        service_stream_provider_with_deferred_start
          ( boost::asio::io_service& io_service
          , service_dispatcher& dispatcher
          , typename Protocol::endpoint endpoint = {}
          );

        service_stream_provider_with_deferred_start
          ( boost::asio::io_service&
          , std::function< void ( boost::asio::yield_context
                                , boost::archive::binary_iarchive&
                                , boost::archive::binary_oarchive&
                                )
                         > dispatch
          , typename Protocol::endpoint = {}
          );

        //! Start handling requests.
        //! \note Shall only be called once. There is no check to
        //! prevent duplicate calls.
        void start();

        ~service_stream_provider_with_deferred_start();

        //! The endpoint this provider is listening on, either as
        //! specified in the constructor or as automatically assigned.
        //!
        //! \note This endpoint may not be globally uniquely defined,
        //! e.g. UNIX stream sockets are bound to a specific host
        //! which is not included in the endpoint. A TCP/IP endpoint
        //! may use a local address part, not the publicly visible one
        //! (`localhost/::1` vs `example.com`). For the latter \c
        //! util::connectable_to_address_string() can add be used.
        typename Protocol::endpoint local_endpoint() const;

      private:
        boost::asio::io_service& _io_service;
        // \todo Merge, this can't be right. Needs accept/shutdown
        // untangling.
        async_task_termination_guard _async_task_termination_guard_accept;
        async_task_termination_guard _async_task_termination_guard_sockets;

        std::function< void ( boost::asio::yield_context
                            , boost::archive::binary_iarchive&
                            , boost::archive::binary_oarchive&
                            )
                     > _dispatch;
        std::mutex _socket_guard;
        std::list<std::unique_ptr<typename Protocol::socket>> _sockets;

        std::mutex _shutdown_prevention_guard;
        std::size_t _in_progress_dispatches;
        void shutdown_sockets();

        bool _do_accept;
        typename Protocol::acceptor _acceptor;
        typename Protocol::endpoint _local_endpoint;

        void accept_and_start_handler (boost::asio::yield_context);
        void handle ( typename decltype (_sockets)::iterator
                    , boost::asio::yield_context
                    );
      };

      //! \c service_stream_provider is a scoped wrapper of \c
      //! service_stream_provider_with_deferred_start. It implicitly
      //! calls \c service_stream_provider_with_deferred_start::start()
      //! in the constructor and is otherwise equivalent.
      template<typename Protocol, typename Traits>
        struct service_stream_provider
          : service_stream_provider_with_deferred_start<Protocol, Traits>
      {
        service_stream_provider ( boost::asio::io_service&
                                , service_dispatcher&
                                , typename Protocol::endpoint = {}
                                );
        service_stream_provider
          ( boost::asio::io_service&
          , std::function< void ( boost::asio::yield_context
                                , boost::archive::binary_iarchive&
                                , boost::archive::binary_oarchive&
                                )
                         > dispatch
          , typename Protocol::endpoint = {}
          );
      };

      //! The `_with_deferred_dispatcher` variant of \c
      //! service_stream_provider will set up the infrastructure but
      //! will stall handling calls until a dispatcher has been
      //! added. Once a dispatcher is set, future calls are handled
      //! immediately.
      template<typename Protocol, typename Traits>
        struct service_stream_provider_with_deferred_dispatcher
          : public service_stream_provider<Protocol, Traits>
      {
      public:
        service_stream_provider_with_deferred_dispatcher
          (boost::asio::io_service&);

        //! Set the dispatcher to use and wake any stalled calls.
        //! \note Shall be called only once. No check is performed.
        void set_dispatcher (service_dispatcher*);

      private:
        std::mutex _guard;
        std::condition_variable _dispatcher_set;
        service_dispatcher* _dispatcher;
      };
    }
  }
}

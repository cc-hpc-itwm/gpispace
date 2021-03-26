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
      template<typename Protocol, typename Traits>
        struct service_stream_provider_with_deferred_start
      {
      public:
        //! \note The given io_service _has_ to have at least one
        //! thread running. If there are no running threads, behaviour
        //! is undefined (and probably hangs during destructor).
        service_stream_provider_with_deferred_start
          ( boost::asio::io_service&
          , service_dispatcher&
          , typename Protocol::endpoint = {}
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

        //! \note May only be start()ed once. There is no check to
        //! prevent duplicate calls.
        void start();

        ~service_stream_provider_with_deferred_start();

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

      template<typename Protocol, typename Traits>
        struct service_stream_provider_with_deferred_dispatcher
          : public service_stream_provider<Protocol, Traits>
      {
      public:
        service_stream_provider_with_deferred_dispatcher
          (boost::asio::io_service&);

        void set_dispatcher (service_dispatcher*);

      private:
        std::mutex _guard;
        std::condition_variable _dispatcher_set;
        service_dispatcher* _dispatcher;
      };
    }
  }
}

// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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
#include <rpc/remote_endpoint.hpp>

#include <util-generic/serialization/exception.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <condition_variable>
#include <exception>
#include <functional>
#include <list>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template<typename Protocol, typename Traits>
        struct remote_stream_endpoint : remote_endpoint
      {
      public:
        //! \note Spawns operations on strands and is blocking, so may
        //! *not* be used in a `service_handler` when using the same
        //! \a io_service as that handler's server does.
        remote_stream_endpoint
          ( boost::asio::io_service&
          , typename Protocol::endpoint = {}
          , util::serialization::exception::serialization_functions
          = util::serialization::exception::serialization_functions()
          );
        remote_stream_endpoint
          ( boost::asio::io_service&
          , boost::asio::yield_context
          , typename Protocol::endpoint = {}
          , util::serialization::exception::serialization_functions
          = util::serialization::exception::serialization_functions()
          );
        template<typename EndpointIterator>
          remote_stream_endpoint
            ( boost::asio::io_service&
            , EndpointIterator
            , util::serialization::exception::serialization_functions
            = util::serialization::exception::serialization_functions()
            );
        template<typename EndpointIterator>
          remote_stream_endpoint
            ( boost::asio::io_service&
            , boost::asio::yield_context
            , EndpointIterator
            , util::serialization::exception::serialization_functions
            = util::serialization::exception::serialization_functions()
            );
        ~remote_stream_endpoint();

        //! \note Neither \a set_value nor \a set_exception are
        //! allowed to be blocking.
        virtual void send_and_receive
          ( std::vector<char> buffer
          , std::function<void (boost::archive::binary_iarchive&)> set_value
          , std::function<void (std::exception_ptr)> set_exception
          ) override;
        virtual boost::asio::io_service& io_service() override
        {
          return _io_service;
        }

      private:
        void read_responses (boost::asio::yield_context);

        //! \note *May* be called on a strand. Non-blocking.
        template<typename LockGuard1, typename LockGuard2>
          void assume_socket_broken
            ( LockGuard1 const& lock_socket
            , LockGuard2 const& lock_set_exception_and_value
            , std::exception_ptr
            );

        uint64_t _message_counter;

        std::mutex _guard_outgoing;
        boost::upgrade_mutex _guard_socket;
        std::mutex _guard_set_exception_and_value;
        std::unordered_map < uint64_t
                           , std::function<void (boost::archive::binary_iarchive&)>
                           > _set_value;
        std::unordered_map < uint64_t
                           , std::function<void (std::exception_ptr)>
                           > _set_exception;
        struct outgoing
        {
          packet_header header;
          std::vector<char> buffer;
          outgoing (uint64_t message_id, std::vector<char> buffer_)
            : header (message_id, buffer_.size())
            , buffer (std::move (buffer_))
          {}
        };
        std::list<outgoing> _outgoing;
        std::condition_variable _outgoing_handled;

        boost::asio::io_service& _io_service;
        async_task_termination_guard _async_task_termination_guard;
        typename Protocol::socket _socket;
      };
    }
  }
}

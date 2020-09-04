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

#include <rpc/remote_tcp_endpoint.hpp>

#include <rpc/detail/yield_context_with_hooks.hpp>

#include <util-generic/finally.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/thread/reverse_lock.hpp>
#include <boost/thread/shared_lock_guard.hpp>
#include <boost/thread/locks.hpp>

#include <algorithm>
#include <array>
#include <iostream>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      namespace
      {
        template<typename Endpoint>
          struct single_element_endpoint_iter
        {
          single_element_endpoint_iter()
            : _is_end (true)
          {}
          single_element_endpoint_iter (Endpoint endpoint)
            : _is_end (false)
            , _endpoint (std::move (endpoint))
          {}

          Endpoint const& operator*() const
          {
            return _endpoint;
          }
          void operator++()
          {
            _is_end = true;
          }
          bool operator!= (single_element_endpoint_iter<Endpoint> const& other)
          {
            return _is_end != other._is_end;
          }
          bool operator== (single_element_endpoint_iter<Endpoint> const& other)
          {
            return !(_is_end != other._is_end);
          }

          bool _is_end;
          Endpoint _endpoint;
        };
      }

      template<typename Protocol, typename Traits>
        remote_stream_endpoint<Protocol, Traits>::remote_stream_endpoint
          ( boost::asio::io_service& io_service
          , typename Protocol::endpoint endpoint
          , util::serialization::exception::serialization_functions functions
          )
            : remote_stream_endpoint
                ( io_service
                , single_element_endpoint_iter<decltype (endpoint)> (endpoint)
                , std::move (functions)
                )
      {}
      template<typename Protocol, typename Traits>
        remote_stream_endpoint<Protocol, Traits>::remote_stream_endpoint
          ( boost::asio::io_service& io_service
          , boost::asio::yield_context yield
          , typename Protocol::endpoint endpoint
          , util::serialization::exception::serialization_functions functions
          )
            : remote_stream_endpoint
                ( io_service
                , std::move (yield)
                , single_element_endpoint_iter<decltype (endpoint)> (endpoint)
                , std::move (functions)
                )
      {}

      template<typename Protocol, typename Traits> template<typename EndpointIter>
        remote_stream_endpoint<Protocol, Traits>::remote_stream_endpoint
          ( boost::asio::io_service& io_service
          , EndpointIter endpoints
          , util::serialization::exception::serialization_functions functions
          )
        : remote_endpoint (std::move (functions))
        , _message_counter (0)
        , _guard_outgoing()
        , _guard_socket()
        , _guard_set_exception_and_value()
        , _set_value()
        , _set_exception()
        , _outgoing()
        , _outgoing_handled()
        , _io_service (io_service)
        , _async_task_termination_guard()
        , _socket (_io_service)
      {
        boost::asio::connect (_socket, endpoints);

        Traits::apply_socket_options (_socket);

        _async_task_termination_guard.spawn_task
          ( _io_service
          , [this] (boost::asio::yield_context yield)
            {
              read_responses (yield);
            }
          );
      }
      template<typename Protocol, typename Traits> template<typename EndpointIter>
        remote_stream_endpoint<Protocol, Traits>::remote_stream_endpoint
          ( boost::asio::io_service& io_service
          , boost::asio::yield_context yield
          , EndpointIter endpoints
          , util::serialization::exception::serialization_functions functions
          )
        : remote_endpoint (std::move (functions))
        , _message_counter (0)
        , _guard_outgoing()
        , _guard_socket()
        , _guard_set_exception_and_value()
        , _set_value()
        , _set_exception()
        , _outgoing()
        , _outgoing_handled()
        , _io_service (io_service)
        , _async_task_termination_guard()
        , _socket (_io_service)
      {
        boost::asio::async_connect (_socket, endpoints, yield);

        Traits::apply_socket_options (_socket);

        _async_task_termination_guard.spawn_task
          ( _io_service
          , [this] (boost::asio::yield_context yield)
            {
              read_responses (yield);
            }
          );
      }

      template<typename Protocol, typename Traits>
        remote_stream_endpoint<Protocol, Traits>::~remote_stream_endpoint()
      {
        {
          std::lock_guard<boost::upgrade_mutex> const lock_socket
            (_guard_socket);
          std::lock_guard<std::mutex> const lock_set_exception_and_value
            (_guard_set_exception_and_value);

          assume_socket_broken
            ( lock_socket
            , lock_set_exception_and_value
            , std::make_exception_ptr
                (boost::system::system_error (boost::asio::error::shut_down))
            );
        }
        {
          std::unique_lock<std::mutex> lock_outgoing (_guard_outgoing);
          _outgoing_handled.wait
            (lock_outgoing, [this] { return _outgoing.empty(); });
        }

        _async_task_termination_guard.wait_for_termination();

        _socket.close();
      }

      namespace
      {
        template<typename... Lockable>
          yield_context_with_hooks with_unlocked
            (boost::asio::yield_context yield, Lockable&... lockable)
        {
#define CALL(fun_)                                              \
          std::initializer_list<int> {(lockable.fun_(), 0)...};
          return {yield, [&] { CALL (unlock); }, [&] { CALL (lock); }};
#undef CALL
        }
      }

      template<typename Protocol, typename Traits>
        void remote_stream_endpoint<Protocol, Traits>::send_and_receive
          ( std::vector<char> buffer
          , std::function<void (boost::archive::binary_iarchive&)> set_value
          , std::function<void (std::exception_ptr)> set_exception
          )
      {
        {
          std::lock_guard<std::mutex> const lock_outgoing (_guard_outgoing);
          std::lock_guard<std::mutex> const lock_set_exception_and_value
            (_guard_set_exception_and_value);

          uint64_t const message_id (++_message_counter);

          bool has_ongoing_write (!_outgoing.empty());
          _outgoing.emplace_back (message_id, std::move (buffer));

          _set_value.emplace (message_id, std::move (set_value));
          _set_exception.emplace (message_id, std::move (set_exception));

          //! \note async_write does not guarantee non-interleaving writes
          //! when invoked in parallel, thus we explicitly serialize them
          //! by having only one async_write at a time
          if (has_ongoing_write)
          {
            return;
          }
        }

        _async_task_termination_guard.spawn_task
          ( _io_service
          , [this] (boost::asio::yield_context yield)
            {
              // Acquires mutex. Fine: Mutex is never blocking.
              FHG_UTIL_FINALLY ([&] { _outgoing_handled.notify_one(); });

              // Acquires mutex. Fine: Mutex is never blocking.
              std::unique_lock<std::mutex> lock_outgoing (_guard_outgoing);
              // Acquires mutex. Fine: Mutex is never blocking.
              boost::upgrade_lock<boost::upgrade_mutex> shared_lock_socket
                (_guard_socket);

              while (!_outgoing.empty())
              {
                outgoing const& packet (_outgoing.front());

                boost::system::error_code errc;

                {
                  boost::asio::async_write
                    ( _socket
                    , std::array<boost::asio::const_buffer, 2>
                        {{ boost::asio::const_buffer
                             (&packet.header, sizeof (packet.header))
                         , boost::asio::const_buffer
                             (packet.buffer.data(), packet.buffer.size())
                        }}
                    , with_unlocked
                        (yield [errc], lock_outgoing, shared_lock_socket)
                    );
                }

                _outgoing.pop_front();

                if (errc)
                {
                  // Acquires mutex. Fine: Mutex is never blocking.
                  boost::upgrade_to_unique_lock<boost::upgrade_mutex> const
                    lock_socket (shared_lock_socket);
                  // Acquires mutex. Fine: Mutex is never blocking.
                  std::lock_guard<std::mutex> const lock_set_exception_and_value
                    (_guard_set_exception_and_value);

                  assume_socket_broken ( lock_socket
                                       , lock_set_exception_and_value
                                       , std::make_exception_ptr
                                           (boost::system::system_error (errc))
                                       );
                  _outgoing.clear();
                }
              }
            }
          );
      }

      template<typename Protocol, typename Traits>
        void remote_stream_endpoint<Protocol, Traits>::read_responses
          (boost::asio::yield_context yield)
      {
        try
        {
          packet_header response_header;
          std::vector<char> response;
          while (true)
          {
            // Acquires mutex. Fine: Mutex is never blocking.
            boost::shared_lock<boost::upgrade_mutex> lock_socket
              (_guard_socket);

            boost::asio::async_read
              ( _socket
              , boost::asio::buffer (&response_header, sizeof (response_header))
              , with_unlocked (yield, lock_socket)
              );

            response.resize (response_header.buffer_size);

            boost::asio::async_read
              ( _socket
              , boost::asio::buffer (response.data(), response.size())
              , with_unlocked (yield, lock_socket)
              );

            // Acquires mutex. Fine: Mutex is never blocking.
            std::lock_guard<std::mutex> const lock_set_exception_and_value
              (_guard_set_exception_and_value);

            //! \note was assume_broken'd between unyield and lock,
            //! which is fine. _set_exception will also be cleaned up.
            auto const set_value (_set_value.find (response_header.message_id));
            if (set_value == _set_value.end())
            {
              break;
            }

            boost::iostreams::array_source source
              (response.data(), response.size());
            boost::iostreams::stream_buffer<decltype (source)> stream (source);
            boost::archive::binary_iarchive archive (stream);

            // Callback without yield passed down! Fine: only given by
            // `remote_function` which passes a Promise's `set_value`,
            // which is non-blocking.
            set_value->second (archive);
            _set_value.erase (set_value);
            _set_exception.erase (response_header.message_id);
          }
        }
        catch (boost::system::system_error const&)
        {
          // Acquires mutex. Fine: Mutex is never blocking.
          std::lock_guard<boost::upgrade_mutex> const lock_socket (_guard_socket);
          // Acquires mutex. Fine: Mutex is never blocking.
          std::lock_guard<std::mutex> const lock_set_exception_and_value
            (_guard_set_exception_and_value);

          assume_socket_broken
            ( lock_socket
            , lock_set_exception_and_value
            , std::current_exception()
            );
        }
      }

      template<typename Protocol, typename Traits>
        template<typename LockGuard1, typename LockGuard2>
          void remote_stream_endpoint<Protocol, Traits>::assume_socket_broken
            ( LockGuard1 const&
            , LockGuard2 const&
            , std::exception_ptr exception
            )
      {
        //! \note this is mostly to avoid followup requests not failing
        //! (we could never set their promise).
        boost::system::error_code might_already_be_broken_as_something_failed;
        _socket.shutdown ( decltype (_socket)::shutdown_both
                         , might_already_be_broken_as_something_failed
                         );
        _socket.cancel();

        for ( auto const& set_exception
            : _set_exception | boost::adaptors::map_values
            )
        {
          // Callback without yield passed down! Fine: only given by
          // `remote_function` which passes a Promise's
          // `set_exception`, which is non-blocking.
          set_exception (exception);
        }
        _set_value.clear();
        _set_exception.clear();
      }
    }
  }
}

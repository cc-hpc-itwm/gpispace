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

#include <rpc/detail/service_stream_provider.hpp>

#include <rpc/service_dispatcher.hpp>

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <boost/asio/read.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/write.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/asio/use_future.hpp>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template<typename Protocol, typename Traits>
        service_stream_provider_with_deferred_start<Protocol, Traits>
        ::service_stream_provider_with_deferred_start
          ( boost::asio::io_service& io_service
          , service_dispatcher& dispatcher
          , typename Protocol::endpoint endpoint
          )
        : service_stream_provider_with_deferred_start
            ( io_service
            , [&dispatcher] ( boost::asio::yield_context yield
                            , boost::archive::binary_iarchive& input
                            , boost::archive::binary_oarchive& output
                            )
              {
                dispatcher.dispatch (yield, input, output);
              }
            , endpoint
            )
      {}

      template<typename Protocol, typename Traits>
        service_stream_provider_with_deferred_start<Protocol, Traits>
        ::service_stream_provider_with_deferred_start
          ( boost::asio::io_service& io_service
          , std::function< void ( boost::asio::yield_context
                                , boost::archive::binary_iarchive&
                                , boost::archive::binary_oarchive&
                                )
                         > dispatch
          , typename Protocol::endpoint endpoint
          )
        : _io_service (io_service)
        , _async_task_termination_guard_accept()
        , _async_task_termination_guard_sockets()
        , _dispatch (std::move (dispatch))
        , _socket_guard()
        , _sockets()
        , _shutdown_prevention_guard()
        , _in_progress_dispatches (0)
        , _do_accept (true)
        , _acceptor (_io_service, decltype (endpoint) (endpoint))
        , _local_endpoint (_acceptor.local_endpoint())
      {}

      template<typename Protocol, typename Traits>
        void service_stream_provider_with_deferred_start<Protocol, Traits>
        ::start()
      {
        _async_task_termination_guard_accept.spawn_task
          ( _io_service
          , [this] (boost::asio::yield_context yield)
            {
              accept_and_start_handler (yield);
            }
          );
      }

      template<typename Protocol, typename Traits>
        service_stream_provider_with_deferred_start<Protocol, Traits>
        ::~service_stream_provider_with_deferred_start()
      {
        //! \note async_accept() as well as cancel() can't be called
        //! on a closed acceptor, so ensure that all other users of
        //! the acceptor are done before we close the acceptor.

        //! \note We wake up async_accept one more time so that if it
        //! is still running or has just passed a _do_accept that was
        //! still true, it gets out and re-checks _do_accept again,
        //! which will then leave the loop and count down
        //! _async_task_termination_guard_accept. If the thread
        //! already was outside of the loop, our socket will never be
        //! connected, but that's fine as we don't exactly care if it
        //! ever got a connection and we will run through
        //! _async_task_termination_guard_accept straight away. If the
        //! thread never started, _async_task_termination_guard_accept
        //! will be a no-op.

        //! \note Just issuing a `cancel()` does not do the job here
        //! as we can't ensure that the thread didn't yield between
        //! checking _do_accept and async_accept(), so that it might
        //! not yet have posted the request when we're reaching this
        //! point and our cancel() would hit void.

        _do_accept = false;

        //! \note Separate thread as _io_service may not have any
        //! threads associated at all.
        util::scoped_boost_asio_io_service_with_threads io_service (1);
        typename Protocol::socket socket (io_service);
        //! \note Explicitly specify allocator as workaround for
        //! boost.asio issue #112, which lets compilation fail with
        //! libstdc++-5, which no longer internally rebinds the
        //! allocator as needed for allocating the promise.
        //! \todo When bumping boost to a version including the fix
        //! (probably >=1.63), remove workaround.
        //! \see https://github.com/chriskohlhoff/asio/issues/112
        auto connected
          ( socket.async_connect
             (_local_endpoint, boost::asio::use_future[std::allocator<char>{}])
          );

        _async_task_termination_guard_accept.wait_for_termination();

        _acceptor.close();

        //! \note We do not care for the result or what state the
        //! socket is in as we only wanted to trigger an accept.
        socket.close();
        connected.wait();

        shutdown_sockets();

        _async_task_termination_guard_sockets.wait_for_termination();
      }

      template<typename Protocol, typename Traits>
        typename Protocol::endpoint
          service_stream_provider_with_deferred_start<Protocol, Traits>
          ::local_endpoint() const
      {
        return _local_endpoint;
      }

      template<typename Protocol, typename Traits>
        void service_stream_provider_with_deferred_start<Protocol, Traits>
        ::accept_and_start_handler (boost::asio::yield_context yield)
      {
        try
        {
          while (_do_accept)
          {
            auto socket ( util::cxx14::make_unique<typename Protocol::socket>
                            (_io_service)
                        );

            _acceptor.async_accept (*socket, yield);

            Traits::apply_socket_options (*socket);

            typename decltype (_sockets)::iterator socket_it;
            {
              // Acquires mutex. Fine: Mutex is never blocking.
              std::lock_guard<std::mutex> const _ (_socket_guard);
              _sockets.emplace_front (std::move (socket));
              socket_it = _sockets.begin();
            }

            _async_task_termination_guard_sockets.spawn_task
              ( _io_service
              , [this, socket_it] (boost::asio::yield_context inner_yield)
                {
                  handle (socket_it, inner_yield);
                }
              );
          }
        }
        catch (boost::system::system_error const&)
        {
          //! \note ignore all IO errors
          //! \todo don't ignore errors, don't just no longer listen
        }
      }

      template<typename Protocol, typename Traits>
        void service_stream_provider_with_deferred_start<Protocol, Traits>
        ::shutdown_sockets()
      {
        //! \note This lock will never be held over a coroutine yield,
        //! so will not trigger deadlocks.
        //! \note If there are any dispatches in progress, we will
        //! retry in the next io_service cycle. This might busy-poll,
        //! but dispatching shouldn't take too long to avoid
        //! starvation anyway.
        std::lock_guard<std::mutex> const shutdown_prevention_lock
          (_shutdown_prevention_guard);

        if (_in_progress_dispatches)
        {
          return _io_service.post ([&] { shutdown_sockets(); });
        }

        std::lock_guard<std::mutex> const socket_lock (_socket_guard);
        for (auto& socket : _sockets)
        {
          boost::system::error_code might_be_directly_between_catch_and_erase_in_handle;
          socket->shutdown ( Protocol::socket::shutdown_both
                           , might_be_directly_between_catch_and_erase_in_handle
                           );
          socket->cancel (might_be_directly_between_catch_and_erase_in_handle);
        }
      }

      template<typename Protocol, typename Traits>
        void service_stream_provider_with_deferred_start<Protocol, Traits>
        ::handle ( typename decltype (_sockets)::iterator socket_it
                 , boost::asio::yield_context yield
                 )
      {
        auto& socket (*socket_it->get());

        FHG_UTIL_FINALLY
          ( [this, socket_it]
            {
              // Acquires mutex. Fine: Mutex is never blocking.
              std::lock_guard<std::mutex> const _ (_socket_guard);
              boost::system::error_code might_be_in_bad_state_due_to_IO_error;
              (*socket_it)->close (might_be_in_bad_state_due_to_IO_error);
              _sockets.erase (socket_it);
            }
          );

        try
        {
          packet_header request_header;
          std::vector<char> request;
          std::vector<char> response;

          // Endless loop. Fine: yielding inside, getting out via
          // exception.
          while (true)
          {
            boost::asio::async_read
              ( socket
              , boost::asio::buffer (&request_header, sizeof (request_header))
              , yield
              );

            request.resize (request_header.buffer_size);

            {
              //! \note shutdown always holds lock until done, so will
              //! never interleave with us getting the lock. Neither
              //! will ever yield while holding the lock either, so no
              //! coroutine-based deadlocks can occur.

              // Acquires mutex. Fine: Mutex is never blocking.
              std::lock_guard<std::mutex> const _ (_shutdown_prevention_guard);
              ++_in_progress_dispatches;

              //! \note Following this, the socket will not be closed
              //! until all in_progress_dispatches are finished. If
              //! there was a shutdown _before_ we increased the
              //! counter, the following read for the payload will
              //! fail, but the dispatch will not be executed, thus
              //! the client will be notified and no not-rolled-back
              //! call will happen on server side. For the client this
              //! is equivalent to the server not existing
              //! anymore. For the server, this is equivalent to the
              //! client never calling to begin with. If the shutdown
              //! is tried _after_ we increased the counter, it will
              //! be deferred until all dispatches are finished.
            }
            FHG_UTIL_FINALLY
              ( [&]
                {
                  // Acquires mutex. Fine: Mutex is never blocking.
                  std::lock_guard<std::mutex> const _
                    (_shutdown_prevention_guard);
                  --_in_progress_dispatches;
                }
              );

            boost::asio::async_read
              ( socket
              , boost::asio::buffer (request.data(), request.size())
              , yield
              );

            response.clear();

            {
              boost::iostreams::array_source source (request.data(), request.size());
              boost::iostreams::stream_buffer<decltype (source)> input_stream (source);
              boost::archive::binary_iarchive input (input_stream);

              util::vector_sink sink (response);
              boost::iostreams::stream<decltype (sink)> output_stream (sink);
              boost::archive::binary_oarchive output (output_stream);

              // Dangerous: yield is passed down but service_handler
              // allows for not_yielding callbacks. This might be
              // blocking and is not under control!
              //! \todo spawn handling to allow multiple requests from the
              //! same socket in parallel?
              //! \todo pass yield to allow for handling requests from
              //! different sockets in parallel with a single thread
              _dispatch (yield, input, output);
            }

            packet_header const response_header
              (request_header.message_id, response.size());

            //! \todo when write fails, do rollback of some sort
            boost::asio::async_write
              ( socket
              , std::array<boost::asio::const_buffer, 2>
                  {{ boost::asio::const_buffer
                       (&response_header, sizeof (response_header))
                   , boost::asio::const_buffer
                       (response.data(), response.size())
                  }}
              , yield
              );
          }
        }
        catch (boost::system::system_error const&)
        {
          //! \note ignore all IO errors and just kill the socket
        }
      }

      template<typename Protocol, typename Traits>
        service_stream_provider<Protocol, Traits>::service_stream_provider
          ( boost::asio::io_service& io_service
          , service_dispatcher& dispatcher
          , typename Protocol::endpoint endpoint
          )
        : service_stream_provider<Protocol, Traits>
            ( io_service
            , [&dispatcher] ( boost::asio::yield_context yield
                            , boost::archive::binary_iarchive& input
                            , boost::archive::binary_oarchive& output
                            )
              {
                dispatcher.dispatch (yield, input, output);
              }
            , endpoint
            )
      {}

      template<typename Protocol, typename Traits>
        service_stream_provider<Protocol, Traits>::service_stream_provider
          ( boost::asio::io_service& io_service
          , std::function< void ( boost::asio::yield_context
                                , boost::archive::binary_iarchive&
                                , boost::archive::binary_oarchive&
                                )
                         > dispatch
          , typename Protocol::endpoint endpoint
          )
        : service_stream_provider_with_deferred_start<Protocol, Traits>
            (io_service, std::move (dispatch), std::move (endpoint))
      {
        service_stream_provider_with_deferred_start<Protocol, Traits>::start();
      }

      template<typename Protocol, typename Traits>
        service_stream_provider_with_deferred_dispatcher<Protocol, Traits>
        ::service_stream_provider_with_deferred_dispatcher
          (boost::asio::io_service& io_service)
        : service_stream_provider<Protocol, Traits>
             ( io_service
             , [this] ( boost::asio::yield_context yield
                      , boost::archive::binary_iarchive& input
                      , boost::archive::binary_oarchive& output
                      )
               {
                 std::unique_lock<std::mutex> lock (_guard);
                 _dispatcher_set.wait (lock, [&] { return _dispatcher; });

                 _dispatcher->dispatch (yield, input, output);
               }
             )
         , _dispatcher (nullptr)
       {}

      template<typename Protocol, typename Traits>
        void service_stream_provider_with_deferred_dispatcher<Protocol, Traits>
        ::set_dispatcher (service_dispatcher* dispatcher)
      {
        {
          std::unique_lock<std::mutex> const _ (_guard);
          assert (!_dispatcher);
          _dispatcher = dispatcher;
        }
        _dispatcher_set.notify_all();
      }
    }
  }
}

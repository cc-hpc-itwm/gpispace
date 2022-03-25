// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_socket_endpoint.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_socket_provider.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/warning.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <future>
#include <initializer_list>
#include <limits>
#include <memory>
#include <thread>

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (ping, int (int));
}

namespace
{
  std::size_t available_fds()
  {
    //! \note assume 20 magically lost file descriptors (stdio, boost
    //! asio internal (epoll), ...)
    return fhg::util::suppress_warning::sign_conversion<std::size_t>
      ( fhg::util::syscall::sysconf (_SC_OPEN_MAX)
      , "-1 = infinite = 2^64 -> number of clients limited by ports"
      ) - 20;
  }

  std::size_t available_processes()
  {
    //! \note assume 2000 magically lost processes (probably a low
    //! bound, seeing that even browsers spawn a process/thread per
    //! tab nowadays).
    return fhg::util::suppress_warning::sign_conversion<std::size_t>
      ( fhg::util::syscall::sysconf (_SC_CHILD_MAX)
      , "-1 = infinite = 2^64 -> number of threads spawned limited by fds/ports"
      ) - 2000;
  }

  std::size_t available_ports()
  {
    //! \note assume 1024 lost due to not running as root, and another
    //! 1024 as buffer for other processes, like the cluster manager.
    return std::numeric_limits<uint16_t>::max() - 1024 * 2;
  }

  std::size_t available_autobind_sockets()
  {
    //! \note The actual limit is around 2^20 according to the internet,
    //! but I can't find a value in our 2008 kernel man pages, or a
    //! ulimit. It is very likely greater or equal to the number of
    //! ports due to there being UNIX_PATH_MAX-1 (107) characters for
    //! use in sockaddr_un.sun_path. Simply printing it shows that our
    //! kernel uses a 5 digit base10 number, thus allowing for 99999
    //! anonymous sockets. Due to this all being handweaving, just
    //! assume the same limit as for ports.
    return available_ports();
  }

  std::size_t max_client_count_tcp()
  {
    //! \note assume 1 fd for server accepting, 2 fds per connection
    //! (server + client socket), limit to available ports at most
    return std::min (available_fds() - 1, available_ports()) / 2;
  }

  std::size_t max_client_count_socket()
  {
    //! \note assume 1 fd for server accepting, 2 fds per connection
    //! (server + client socket), limit to available autobind sockets at most
    return std::min (available_fds() - 1, available_autobind_sockets()) / 2;
  }

  std::size_t max_client_thread_count_tcp()
  {
    return std::min (max_client_count_tcp(), available_processes());
  }
  std::size_t max_client_thread_count_socket()
  {
    return std::min (max_client_count_socket(), available_processes());
  }
}

namespace data
{
  namespace
  {
    decltype (::boost::unit_test::data::make (std::vector<std::size_t>()))
      values (std::initializer_list<std::size_t> elems)
    {
      return ::boost::unit_test::data::make (std::vector<std::size_t> (elems));
    }
  }
}

BOOST_DATA_TEST_CASE
  ( local_tcp
  , ( data::values ({100, 500, max_client_count_tcp()})
    ^ data::values ({10, 20, max_client_thread_count_tcp()})
    )
  , client_count
  , client_thread_count
  )
{
  BOOST_REQUIRE_LE (client_count, max_client_count_tcp());
  BOOST_REQUIRE_LE (client_thread_count, available_processes());

  std::size_t const rounds (17);

  fhg::util::scoped_boost_asio_io_service_with_threads
    io_service_clients (client_thread_count);
  fhg::util::scoped_boost_asio_io_service_with_threads io_service_server (2);

  fhg::rpc::service_dispatcher service_dispatcher;
  fhg::rpc::service_handler<protocol::ping> start_service
    ( service_dispatcher
    , [] (int i)
      {
        std::this_thread::sleep_for (std::chrono::microseconds (100));
        return i + 1;
      }
    );
  fhg::rpc::service_tcp_provider const server
    (io_service_server, service_dispatcher);
  std::string const endpoint_address
    ( fhg::util::connectable_to_address_string
        (server.local_endpoint().address())
    );

  std::vector<std::unique_ptr<fhg::rpc::remote_tcp_endpoint>> clients;
  std::vector<std::future<int>> futures;

  for (std::size_t i (0); i < client_count; ++i)
  {
    clients.emplace_back
      ( std::make_unique<fhg::rpc::remote_tcp_endpoint>
          (io_service_clients, endpoint_address, server.local_endpoint().port())
      );
    futures.emplace_back
      (fhg::rpc::remote_function<protocol::ping> {*clients.back()} (i));
  }

  std::vector<std::future<int>> futures2;

  std::vector<std::future<int>>* current (&futures);
  std::vector<std::future<int>>* next (&futures2);

  std::size_t round (0);
  for (; round < rounds; ++round)
  {
    for (std::size_t i (0); i < client_count; ++i)
    {
      BOOST_REQUIRE_EQUAL (current->at (i).get(), round + i + 1);
      next->emplace_back
        ( fhg::rpc::remote_function<protocol::ping> {*clients.at (i)}
            (round + 1 + i)
        );
    }
    current->clear();
    std::swap (current, next);
  }

  for (std::size_t i (0); i < client_count; ++i)
  {
    BOOST_REQUIRE_EQUAL (current->at (i).get(), round + i + 1);
  }
}

BOOST_DATA_TEST_CASE
  ( local_socket
  , ( data::values ({100, 500, max_client_count_tcp()})
    ^ data::values ({10, 20, max_client_thread_count_socket()})
    )
  , client_count
  , client_thread_count
  )
{
  BOOST_REQUIRE_LE (client_count, max_client_count_socket());
  BOOST_REQUIRE_LE (client_thread_count, available_processes());

  std::size_t const rounds (17);

  fhg::util::scoped_boost_asio_io_service_with_threads
    io_service_clients (client_thread_count);
  fhg::util::scoped_boost_asio_io_service_with_threads io_service_server (2);

  fhg::rpc::service_dispatcher service_dispatcher;
  fhg::rpc::service_handler<protocol::ping> start_service
    ( service_dispatcher
    , [] (int i)
      {
        std::this_thread::sleep_for (std::chrono::microseconds (100));
        return i + 1;
      }
    );
  fhg::rpc::service_socket_provider const server
    (io_service_server, service_dispatcher);

  std::vector<std::unique_ptr<fhg::rpc::remote_socket_endpoint>> clients;
  std::vector<std::future<int>> futures;

  for (std::size_t i (0); i < client_count; ++i)
  {
    clients.emplace_back
      ( std::make_unique<fhg::rpc::remote_socket_endpoint>
          (io_service_clients, server.local_endpoint())
      );
    futures.emplace_back
      (fhg::rpc::remote_function<protocol::ping> {*clients.back()} (i));
  }

  std::vector<std::future<int>> futures2;

  std::vector<std::future<int>>* current (&futures);
  std::vector<std::future<int>>* next (&futures2);

  std::size_t round (0);
  for (; round < rounds; ++round)
  {
    for (std::size_t i (0); i < client_count; ++i)
    {
      BOOST_REQUIRE_EQUAL (current->at (i).get(), round + i + 1);
      next->emplace_back
        ( fhg::rpc::remote_function<protocol::ping> {*clients.at (i)}
            (round + 1 + i)
        );
    }
    current->clear();
    std::swap (current, next);
  }

  for (std::size_t i (0); i < client_count; ++i)
  {
    BOOST_REQUIRE_EQUAL (current->at (i).get(), round + i + 1);
  }
}

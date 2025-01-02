// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_socket_endpoint.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_socket_provider.hpp>

#include <util-generic/latch.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/temporary_file.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/future.hpp>
#include <util-generic/testing/printer/tuple.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/serialization/list.hpp>
#include <boost/test/unit_test.hpp>

//! \todo cross-process test cases?

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (ping, int (int));
}

BOOST_AUTO_TEST_CASE (int_ping)
{
  fhg::rpc::service_dispatcher service_dispatcher;
  fhg::rpc::service_handler<protocol::ping> start_service
    ( service_dispatcher
    , [] (int i) { return i + 1; }
    );
  fhg::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  fhg::rpc::service_socket_provider const server
    {io_service_server, service_dispatcher};

  fhg::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
  fhg::rpc::remote_socket_endpoint client
    (io_service_client, server.local_endpoint());

  int const s (fhg::util::testing::random<int>{}());
  std::future<int> pong (fhg::rpc::remote_function<protocol::ping> (client) (s));

  BOOST_REQUIRE_EQUAL ( pong.wait_for (std::chrono::seconds (10))
                      , std::future_status::ready
                      );

  BOOST_REQUIRE_EQUAL (pong.get(), s + 1);
}

BOOST_AUTO_TEST_CASE (slow_parallel_pings)
{
  fhg::util::scoped_boost_asio_io_service_with_threads io_service (10);

  fhg::util::latch function_calls_placed (1);

  fhg::rpc::service_dispatcher service_dispatcher;
  fhg::rpc::service_handler<protocol::ping> start_service
    ( service_dispatcher
    , [&function_calls_placed] (int i)
      {
        // Blocking. Fine: single-threaded io_service only used by
        // server.
        function_calls_placed.wait();
        return i + 1;
      }
    );
  fhg::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  fhg::rpc::service_socket_provider const server
    {io_service_server, service_dispatcher};

  fhg::rpc::remote_socket_endpoint client
    (io_service, server.local_endpoint());

  fhg::rpc::remote_function<protocol::ping> ping (client);

  std::vector<std::future<int>> futures;
  for (int i (0); i < 10000; ++i)
  {
    futures.emplace_back (ping (i));
  }

  function_calls_placed.count_down();

  int i (0);
  for (auto& future : futures)
  {
    BOOST_REQUIRE_EQUAL (future.get(), ++i);
  }
}

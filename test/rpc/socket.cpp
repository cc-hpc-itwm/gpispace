#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_socket_endpoint.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_socket_provider.hpp>

#include <gspc/util/latch.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/future.hpp>
#include <gspc/testing/printer/tuple.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/serialization/list.hpp>
#include <boost/test/unit_test.hpp>

//! \todo cross-process test cases?

namespace protocol
{
  FHG_RPC_FUNCTION_DESCRIPTION (ping, int (int));
}

BOOST_AUTO_TEST_CASE (int_ping)
{
  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::ping> start_service
    ( service_dispatcher
    , [] (int i) { return i + 1; }
    );
  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  gspc::rpc::service_socket_provider const server
    {io_service_server, service_dispatcher};

  gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
  gspc::rpc::remote_socket_endpoint client
    (io_service_client, server.local_endpoint());

  int const s (gspc::testing::random<int>{}());
  std::future<int> pong (gspc::rpc::remote_function<protocol::ping> (client) (s));

  BOOST_REQUIRE_EQUAL ( pong.wait_for (std::chrono::seconds (10))
                      , std::future_status::ready
                      );

  BOOST_REQUIRE_EQUAL (pong.get(), s + 1);
}

BOOST_AUTO_TEST_CASE (slow_parallel_pings)
{
  gspc::util::scoped_boost_asio_io_service_with_threads io_service (10);

  gspc::util::latch function_calls_placed (1);

  gspc::rpc::service_dispatcher service_dispatcher;
  gspc::rpc::service_handler<protocol::ping> start_service
    ( service_dispatcher
    , [&function_calls_placed] (int i)
      {
        // Blocking. Fine: single-threaded io_service only used by
        // server.
        function_calls_placed.wait();
        return i + 1;
      }
    );
  gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
  gspc::rpc::service_socket_provider const server
    {io_service_server, service_dispatcher};

  gspc::rpc::remote_socket_endpoint client
    (io_service, server.local_endpoint());

  gspc::rpc::remote_function<protocol::ping> ping (client);

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

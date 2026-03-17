#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_socket_endpoint.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_socket_provider.hpp>

#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <atomic>
#include <cmath>
#include <cstddef>
#include <string>


  namespace gspc::rpc
  {
    namespace protocol
    {
      template<typename T>
        FHG_RPC_FUNCTION_DESCRIPTION (ping, T (T));
    }

    BOOST_AUTO_TEST_CASE (instantiations_need_to_be_done_manually)
    {
      service_dispatcher dispatcher;
      service_handler<protocol::ping<int>> ping_int_service
        (dispatcher, [&] (int i) { return i; });
      // no ping_string_service!

      gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
      service_socket_provider const server {io_service_server, dispatcher};

      gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
      remote_socket_endpoint client (io_service_client, server.local_endpoint());

      auto const random_int (gspc::testing::random<int>{}());
      BOOST_REQUIRE_EQUAL
        ( sync_remote_function<protocol::ping<int>> (client) (random_int)
        , random_int
        );

      gspc::testing::require_exception
        ( [&]
          {
            sync_remote_function<protocol::ping<std::string>> {client} ("");
          }
        , error::unknown_function (typeid (protocol::ping<std::string>).name())
        );
    }

    BOOST_AUTO_TEST_CASE (multiple_instances_are_possible)
    {
      std::atomic<std::size_t> calls_to_ping_int {0};
      std::atomic<std::size_t> calls_to_ping_float {0};

      service_dispatcher dispatcher;
      service_handler<protocol::ping<int>> ping_int_service
        ( dispatcher
        , [&] (int i)
          {
            ++calls_to_ping_int;
            return i;
          }
        );
      service_handler<protocol::ping<float>> ping_float_service
        ( dispatcher
        , [&] (float i)
          {
            ++calls_to_ping_float;
            return i;
          }
        );

      gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
      service_socket_provider const server {io_service_server, dispatcher};

      gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
      remote_socket_endpoint client (io_service_client, server.local_endpoint());

      auto const random_int (gspc::testing::random<int>{}());
      auto const random_float (gspc::testing::random<float>{}());

      BOOST_REQUIRE_EQUAL (calls_to_ping_int, 0);
      BOOST_REQUIRE_EQUAL (calls_to_ping_float, 0);

      auto const pong_int
        (sync_remote_function<protocol::ping<int>> {client} (random_int));

      BOOST_REQUIRE_EQUAL (calls_to_ping_int, 1);
      BOOST_REQUIRE_EQUAL (calls_to_ping_float, 0);
      BOOST_REQUIRE_EQUAL (pong_int, random_int);

      auto const pong_float
        (sync_remote_function<protocol::ping<float>> {client} (random_float));

      BOOST_REQUIRE_EQUAL (calls_to_ping_int, 1);
      BOOST_REQUIRE_EQUAL (calls_to_ping_float, 1);
      BOOST_REQUIRE_EQUAL (pong_float, random_float);
    }

    namespace protocol
    {
      template<typename T>
        FHG_RPC_FUNCTION_DESCRIPTION (sin, T (T));

      namespace
      {
        float sinus (float x)
        {
          return std::sin (x);
        }

        double sinus (double x)
        {
          return std::sin (x);
        }
      }
    }

    BOOST_AUTO_TEST_CASE (overload_function_with_templates)
    {
      service_dispatcher dispatcher;
      service_handler<protocol::sin<float>> const sin_float
        (dispatcher, static_cast<float(*) (float)> (protocol::sinus));
      service_handler<protocol::sin<double>> const sin_double
        (dispatcher, static_cast<double(*) (double)> (protocol::sinus));

      gspc::util::scoped_boost_asio_io_service_with_threads io_service_server (1);
      service_socket_provider const server {io_service_server, dispatcher};

      gspc::util::scoped_boost_asio_io_service_with_threads io_service_client (1);
      remote_socket_endpoint client (io_service_client, server.local_endpoint());

      auto const random_float (gspc::testing::random<float>{}());
      auto const random_double (gspc::testing::random<double>{}());

      BOOST_REQUIRE_EQUAL
        ( sync_remote_function<protocol::sin<float>> {client} (random_float)
        , std::sin (random_float)
        );

      BOOST_REQUIRE_EQUAL
        ( sync_remote_function<protocol::sin<double>> {client} (random_double)
        , std::sin (random_double)
        );
    }
  }

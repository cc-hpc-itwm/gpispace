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

#include <rpc/remote_function.hpp>
#include <rpc/remote_socket_endpoint.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_socket_provider.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <atomic>
#include <cmath>
#include <cstddef>
#include <string>

namespace fhg
{
  namespace rpc
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

      util::scoped_boost_asio_io_service_with_threads io_service_server (1);
      service_socket_provider const server {io_service_server, dispatcher};

      util::scoped_boost_asio_io_service_with_threads io_service_client (1);
      remote_socket_endpoint client (io_service_client, server.local_endpoint());

      auto const random_int (util::testing::random<int>{}());
      BOOST_REQUIRE_EQUAL
        ( sync_remote_function<protocol::ping<int>> (client) (random_int)
        , random_int
        );

      util::testing::require_exception
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

      util::scoped_boost_asio_io_service_with_threads io_service_server (1);
      service_socket_provider const server {io_service_server, dispatcher};

      util::scoped_boost_asio_io_service_with_threads io_service_client (1);
      remote_socket_endpoint client (io_service_client, server.local_endpoint());

      auto const random_int (util::testing::random<int>{}());
      auto const random_float (util::testing::random<float>{}());

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

      util::scoped_boost_asio_io_service_with_threads io_service_server (1);
      service_socket_provider const server {io_service_server, dispatcher};

      util::scoped_boost_asio_io_service_with_threads io_service_client (1);
      remote_socket_endpoint client (io_service_client, server.local_endpoint());

      auto const random_float (util::testing::random<float>{}());
      auto const random_double (util::testing::random<double>{}());

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
}

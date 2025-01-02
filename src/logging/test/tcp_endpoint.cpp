// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <logging/tcp_endpoint.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

namespace fhg
{
  namespace logging
  {
    namespace
    {
      std::string random_host()
      {
        std::string result;
        do
        {
          // ':': separator to port
          result = util::testing::random_string_without (":");
        }
        while (result.empty());
        return result;
      }
    }

    BOOST_AUTO_TEST_CASE (constructible_from_host_and_port)
    {
      auto const host (random_host());
      auto const port (util::testing::random<unsigned short>{}());

      tcp_endpoint const endpoint (host, port);

      BOOST_REQUIRE_EQUAL (endpoint.host, host);
      BOOST_REQUIRE_EQUAL (endpoint.port, port);
    }

    BOOST_AUTO_TEST_CASE (constructible_from_host_and_port_pair)
    {
      auto const host (random_host());
      auto const port (util::testing::random<unsigned short>{}());

      tcp_endpoint const endpoint (std::make_pair (host, port));

      BOOST_REQUIRE_EQUAL (endpoint.host, host);
      BOOST_REQUIRE_EQUAL (endpoint.port, port);
    }

    BOOST_AUTO_TEST_CASE (constructible_from_host_and_port_string)
    {
      auto const host (random_host());
      auto const port (util::testing::random<unsigned short>{}());

      tcp_endpoint const endpoint (host + ":" + std::to_string (port));

      BOOST_REQUIRE_EQUAL (endpoint.host, host);
      BOOST_REQUIRE_EQUAL (endpoint.port, port);
    }

    BOOST_DATA_TEST_CASE
      ( constructing_from_bad_string_throws
      , ::boost::unit_test::data::make
          ( { "" // empty
            , "onlyhost" // no colon
            , ":" // only colon
            , "host:" // colon but no port
            , ":0" // colon but no host
            , "host:port" // port is not a number
            , "host:0port" // port is not only a number
            , "host:-1" // port is signed
            , "host:65537" // port is not fitting in an unsigned short
            }
          )
      , input
      )
    {
      util::testing::require_exception
        ( [&] { tcp_endpoint {input}; }
        , error::bad_host_and_port_string (input)
        );
    }
  }
}

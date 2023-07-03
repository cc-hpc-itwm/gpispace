// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <logging/socket_endpoint.hpp>

#include <util-generic/hostname.hpp>
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
          // ':': separator to path
          result = util::testing::random_string_without (":");
        }
        while (result.empty());
        return result;
      }
      std::string random_socket (std::size_t chop_off = 0)
      {
        assert (chop_off < sizeof (sockaddr_un::sun_path));
        std::string result;
        do
        {
          // '\0': ends up being a C string parser somewhere deep below
          result = util::testing::random_string_without_zero();
        }
        while ( result.size() >= sizeof (sockaddr_un::sun_path) - chop_off
             || result.empty()
              );
        return result;
      }
    }

    BOOST_AUTO_TEST_CASE (constructible_from_host_and_socket)
    {
      auto const host (random_host());
      auto const socket (random_socket());

      socket_endpoint const endpoint (host, socket);

      BOOST_REQUIRE_EQUAL (endpoint.host, host);
      BOOST_REQUIRE_EQUAL (endpoint.socket.path(), socket);
    }

    BOOST_AUTO_TEST_CASE (constructible_from_host_and_socket_pair)
    {
      auto const host (random_host());
      auto const socket (random_socket());

      socket_endpoint const endpoint (std::make_pair (host, socket));

      BOOST_REQUIRE_EQUAL (endpoint.host, host);
      BOOST_REQUIRE_EQUAL (endpoint.socket.path(), socket);
    }

    BOOST_AUTO_TEST_CASE (constructible_from_host_and_socket_string)
    {
      auto const host (random_host());
      auto const socket (random_socket());

      socket_endpoint const endpoint (host + ":" + socket);

      BOOST_REQUIRE_EQUAL (endpoint.host, host);
      BOOST_REQUIRE_EQUAL (endpoint.socket, socket);
    }

    BOOST_AUTO_TEST_CASE (auto_generated_sockets_use_explicit_null_char)
    {
      auto const path (random_socket (1));
      socket_endpoint::Socket const raw (std::string (1, '\0') + path);

      std::string const host (fhg::util::hostname());

      socket_endpoint const from_local (raw);
      BOOST_REQUIRE_EQUAL (from_local.to_string(), host + ":\\0" + path);

      socket_endpoint const from_host_and_socket (host, raw);
      BOOST_REQUIRE_EQUAL
        (from_local.to_string(), from_host_and_socket.to_string());

      socket_endpoint const from_string (host + ":\\0" + path);
      BOOST_REQUIRE_EQUAL (from_local.to_string(), from_string.to_string());
    }

    BOOST_DATA_TEST_CASE
      ( constructing_from_bad_string_throws
      , ::boost::unit_test::data::make
          ( { "" // empty
            , "onlyhost" // no colon
            , ":" // only colon
            , "host:" // colon but no socket
            , ":0" // colon but no host
            }
          )
      , input
      )
    {
      util::testing::require_exception
        ( [&] { socket_endpoint {std::string (input)}; }
        , error::bad_host_and_socket_string (input)
        );
    }
  }
}

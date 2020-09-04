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

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace ip = boost::asio::ip;

    BOOST_AUTO_TEST_CASE (port_is_passed_through)
    {
      auto const port (testing::random<std::uint16_t>{}());
      BOOST_REQUIRE_EQUAL ( connectable_to_address_string
                              (ip::tcp::endpoint {ip::address{}, port}).second
                          , port
                          );
    }

    BOOST_AUTO_TEST_CASE (unspecified_host_defaults_to_hostname)
    {
      BOOST_REQUIRE_EQUAL
        (connectable_to_address_string (ip::address{}), hostname());
      BOOST_REQUIRE_EQUAL ( connectable_to_address_string
                              (ip::tcp::endpoint {ip::address{}, {}}).first
                          , hostname()
                          );
    }

    BOOST_AUTO_TEST_CASE (specified_host_is_passed_through)
    {
      std::uint32_t const host0 (testing::random<std::uint8_t>{}());
      std::uint32_t const host1 (testing::random<std::uint8_t>{}());
      std::uint32_t const host2 (testing::random<std::uint8_t>{}());
      std::uint32_t const host3 (testing::random<std::uint8_t>{}());
      std::uint32_t const host (host0 | host1 << 8 | host2 << 16 | host3 << 24);
      std::string const host_string
        ( std::to_string (host3) + "." + std::to_string (host2) + "."
        + std::to_string (host1) + "." + std::to_string (host0)
        );

      BOOST_REQUIRE_EQUAL
        ( connectable_to_address_string (ip::address {ip::address_v4 {host}})
        , host_string
        );
      BOOST_REQUIRE_EQUAL
        ( connectable_to_address_string
            (ip::tcp::endpoint {ip::address {ip::address_v4 {host}}, {}}).first
        , host_string
        );
    }
  }
}

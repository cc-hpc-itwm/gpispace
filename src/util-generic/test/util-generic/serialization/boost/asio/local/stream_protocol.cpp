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

#include <util-generic/serialization/boost/asio/local/stream_protocol.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/asio/local/stream_protocol.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (path_is_passed_through)
{
  //! \note There is no public exposure from boost, sadly. This
  //! constant is taken from `endpoint::init()` in
  //! boost/1.61.0/include/boost/asio/local/detail/impl/endpoint.ipp.
  constexpr auto const max_length
    = sizeof (::boost::asio::detail::sockaddr_un_type::sun_path) - 1;

  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ( ( ::boost::asio::local::stream_protocol::endpoint
          {fhg::util::testing::random<std::string>{}().substr (0, max_length)}
      )
    , ::boost::asio::local::stream_protocol::endpoint
    );
}

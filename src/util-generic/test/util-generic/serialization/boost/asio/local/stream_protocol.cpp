// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/first_then.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

BOOST_AUTO_TEST_CASE (first_then_works)
{
  std::string const first {fhg::util::testing::random<std::string>{}()};
  std::string const then {fhg::util::testing::random<std::string>{}()};

  fhg::util::first_then<std::string> const first_then {first, then};

  BOOST_REQUIRE_EQUAL (first_then.string(), first);
  BOOST_REQUIRE_EQUAL (first_then.string(), then);
  BOOST_REQUIRE_EQUAL (first_then.string(), then);
}

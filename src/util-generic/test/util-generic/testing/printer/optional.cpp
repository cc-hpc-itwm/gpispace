// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/optional.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (unset)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("Nothing", ::boost::none);
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("Nothing", ::boost::optional<int>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("Nothing", ::boost::optional<std::string>{});
}

BOOST_AUTO_TEST_CASE (set)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("Just 1", ::boost::optional<int> {1});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("Just foo", ::boost::optional<std::string> {"foo"});
}

// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/set.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("set {}", std::set<std::string>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("set {}", std::set<int>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("set {}", std::set<float>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("set {foo}", std::set<std::string> {"foo"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("set {1}", std::set<int> {1});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("set {1}", std::set<float> {1.0f});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("set {bar, baz, foo}", std::set<std::string> {"foo", "bar", "baz"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("set {1, 63, 4523}", std::set<int> {1, 63, 4523});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("set {1, 2, 16.3999996}", std::set<float> {1.0f, 2.0f, 16.4f});
}

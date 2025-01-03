// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/multiset.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("multiset {}", std::multiset<std::string>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("multiset {}", std::multiset<int>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("multiset {}", std::multiset<float>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multiset {foo}", std::multiset<std::string> {"foo"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multiset {1}", std::multiset<int> {1});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multiset {1}", std::multiset<float> {1.0f});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multiset {bar, baz, foo}", std::multiset<std::string> {"foo", "bar", "baz"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multiset {1, 63, 4523}", std::multiset<int> {1, 63, 4523});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multiset {1, 2, 16.3999996}", std::multiset<float> {1.0f, 2.0f, 16.4f});
}

BOOST_AUTO_TEST_CASE (multiple_elements_same_key)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multiset {bar, bar, bar}", std::multiset<std::string> {"bar", "bar", "bar"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multiset {63, 63, 63}", std::multiset<int> {63, 63, 63});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "multiset {16.3999996, 16.3999996, 16.3999996}"
    , std::multiset<float> {16.4f, 16.4f, 16.4f}
    );
}

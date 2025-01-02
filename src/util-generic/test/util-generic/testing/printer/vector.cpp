// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/vector.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("vector ()", std::vector<std::string>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("vector ()", std::vector<int>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("vector ()", std::vector<float>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("vector (foo)", std::vector<std::string> {"foo"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("vector (1)", std::vector<int> {1});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("vector (1)", std::vector<float> {1.0f});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("vector (foo, bar, baz)", std::vector<std::string> {"foo", "bar", "baz"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("vector (1, 63, 4523)", std::vector<int> {1, 63, 4523});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("vector (1, 2, 16.3999996)", std::vector<float> {1.0f, 2.0f, 16.4f});
}

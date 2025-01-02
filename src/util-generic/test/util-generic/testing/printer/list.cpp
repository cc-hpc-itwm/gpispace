// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/list.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("list ()", std::list<std::string>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("list ()", std::list<int>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("list ()", std::list<float>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("list (foo)", std::list<std::string> {"foo"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("list (1)", std::list<int> {1});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("list (1)", std::list<float> {1.0f});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("list (foo, bar, baz)", std::list<std::string> {"foo", "bar", "baz"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("list (1, 63, 4523)", std::list<int> {1, 63, 4523});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("list (1, 2, 16.3999996)", std::list<float> {1.0f, 2.0f, 16.4f});
}

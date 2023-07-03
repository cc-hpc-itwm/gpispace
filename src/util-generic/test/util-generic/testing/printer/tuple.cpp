// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/tuple.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("<>", std::tuple<>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("<foo>", std::tuple<std::string> {"foo"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("<1>", std::tuple<int> {1});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("<1>", std::tuple<float> {1.0f});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("<foo, bar, baz>", std::make_tuple ("foo", "bar", "baz"));
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("<1, 63, 4523>", std::make_tuple (1, 63, 4523));
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("<1, 2, 16.3999996>", std::make_tuple (1.0f, 2.0f, 16.4f));

  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("<foo, 63, 16.3999996>", std::make_tuple ("foo", 63, 16.4f));
}

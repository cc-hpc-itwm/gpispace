// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/printer/pair.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("<foo, bar>", std::make_pair ("foo", "bar"));
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("<1, 63>", std::make_pair (1, 63));
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("<1, 2>", std::make_pair (1.0f, 2.0f));

  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("<foo, 16.3999996>", std::make_pair ("foo", 16.4f));
}

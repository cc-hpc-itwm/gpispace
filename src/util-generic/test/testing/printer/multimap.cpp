// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/test/testing/printer/require_printed_as.hpp>
#include <util-generic/testing/printer/multimap.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("multimap []", std::multimap<std::string, int>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("multimap []", std::multimap<int, int>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("multimap []", std::multimap<float, int>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multimap [<foo, 1>]", std::multimap<std::string, int> {{"foo", 1}});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multimap [<1, 16.3999996>]", std::multimap<int, float> {{1, 16.4f}});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("multimap [<1, bar>]", std::multimap<float, std::string> {{1.0f, "bar"}});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "multimap [<bar, 1>, <baz, 2>]"
    , std::multimap<std::string, int> {{"baz", 2}, {"bar", 1}}
    );
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "multimap [<1, 16.3999996>, <63, 1>]"
    , std::multimap<int, float> {{1, 16.4f}, {63, 1.0f}}
    );
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "multimap [<1, bar>, <2, baz>]"
    , std::multimap<float, std::string> {{2.0f, "baz"}, {1.0f, "bar"}}
    );
}

BOOST_AUTO_TEST_CASE (multiple_elements_with_same_key)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "multimap [<baz, 2>, <baz, 2>]"
    , std::multimap<std::string, int> {{"baz", 2}, {"baz", 2}}
    );
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "multimap [<2, 16.3999996>, <2, 1>, <2, 0.5>]"
    , std::multimap<int, float> {{2, 16.4f}, {2, 1.0f}, {2, 0.5f}}
    );
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "multimap [<2, baz>, <2, bar>]"
    , std::multimap<float, std::string> {{2.0f, "baz"}, {2.0f, "bar"}}
    );
}

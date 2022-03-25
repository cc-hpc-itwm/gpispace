// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-generic/testing/printer/map.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("map []", std::map<std::string, int>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("map []", std::map<int, int>{});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("map []", std::map<float, int>{});
}

BOOST_AUTO_TEST_CASE (one_element)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("map [<foo, 1>]", std::map<std::string, int> {{"foo", 1}});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("map [<1, 16.3999996>]", std::map<int, float> {{1, 16.4f}});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("map [<1, bar>]", std::map<float, std::string> {{1.0f, "bar"}});
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "map [<bar, 1>, <baz, 2>]"
    , std::map<std::string, int> {{"baz", 2}, {"bar", 1}}
    );
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "map [<1, 16.3999996>, <63, 1>]"
    , std::map<int, float> {{1, 16.4f}, {63, 1.0f}}
    );
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ( "map [<1, bar>, <2, baz>]"
    , std::map<float, std::string> {{2.0f, "baz"}, {1.0f, "bar"}}
    );
}

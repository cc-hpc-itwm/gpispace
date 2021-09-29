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

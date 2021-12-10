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

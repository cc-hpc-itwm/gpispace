// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/serialization/std/tuple.hpp>

#include <util-generic/testing/printer/tuple.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({}, std::tuple<>);
}

BOOST_AUTO_TEST_CASE (one_element)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({"foo"}, std::tuple<std::string>);
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({1}, std::tuple<int>);
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({1.0f}, std::tuple<float>);
}

BOOST_AUTO_TEST_CASE (multiple_elements)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    (("foo", "bar", "baz"), std::tuple<std::string, std::string, std::string>);
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ((1, 63, 4523), std::tuple<int, int, int>);
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ((1.0f, 2.0f, 16.4f), std::tuple<float, float, float>);

  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    (("foo", 63, 16.4f), std::tuple<std::string, int, float>);
}

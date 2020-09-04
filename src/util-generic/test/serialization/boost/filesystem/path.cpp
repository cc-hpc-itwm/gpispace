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

#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

BOOST_AUTO_TEST_CASE (empty)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID ({}, boost::filesystem::path);
}

BOOST_AUTO_TEST_CASE (current_path)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({boost::filesystem::current_path()}, boost::filesystem::path);
}

BOOST_AUTO_TEST_CASE (relative)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({"this/is/a/relative/path"}, boost::filesystem::path);
}

BOOST_AUTO_TEST_CASE (absolute)
{
  FHG_UTIL_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({"/this/is/an/absolute/path"}, boost::filesystem::path);
}

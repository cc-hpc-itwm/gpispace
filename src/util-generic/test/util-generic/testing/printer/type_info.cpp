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

#include <util-generic/testing/printer/type_info.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

#include <typeinfo>

BOOST_AUTO_TEST_CASE (std_type_info_name_is_printed)
{
  int i;
  std::type_info const& integer_type (typeid (int));
  std::type_info const& integer_var (typeid (i));
  std::type_info const& integer_ptr (typeid (&i));
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS (integer_type.name(), integer_type);
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS (integer_var.name(), integer_var);
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS (integer_ptr.name(), integer_ptr);
}

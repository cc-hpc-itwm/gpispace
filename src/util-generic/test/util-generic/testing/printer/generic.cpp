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

#include <util-generic/testing/printer/generic.hpp>

#include <util-generic/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

namespace
{
  template<typename T>
    struct templated_type
  {
    T value;
  };

  struct normal_type
  {
    int value;
  };
  std::ostream& operator<< (std::ostream& os, normal_type const& n)
  {
    return os << "*" << n.value;
  }

  struct type_printable_with_wrapper
  {
    int value;
  };
  std::string to_string (type_printable_with_wrapper const& x)
  {
    return "/" + std::to_string (x.value);
  }
}

FHG_BOOST_TEST_LOG_VALUE_PRINTER (normal_type, os, x)
{
  os << "+" << x;
}

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename T>, templated_type<T>, os, x)
{
  os << "-" << x.value;
}

FHG_BOOST_TEST_LOG_VALUE_PRINTER_WRAPPED
  (type_printable_with_wrapper, to_string)

BOOST_AUTO_TEST_CASE (print_normal_type)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("+*1", normal_type {1});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("+*-5", normal_type {-5});
}

BOOST_AUTO_TEST_CASE (print_templated_type)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("-f", templated_type<std::string> {"f"});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("-5", templated_type<int> {5});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS
    ("-*1", templated_type<normal_type> {{1}});
}

BOOST_AUTO_TEST_CASE (print_with_wrapper)
{
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("/1", type_printable_with_wrapper {1});
  FHG_UTIL_TESTING_REQUIRE_PRINTED_AS ("/-5", type_printable_with_wrapper {-5});
}

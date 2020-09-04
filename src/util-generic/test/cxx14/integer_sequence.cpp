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

#include <util-generic/cxx14/integer_sequence.hpp>
#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace cxx14
    {
      BOOST_AUTO_TEST_CASE (size)
      {
        BOOST_REQUIRE_EQUAL ((integer_sequence<int>::size()), 0);
        BOOST_REQUIRE_EQUAL ((integer_sequence<int, 1>::size()), 1);
        BOOST_REQUIRE_EQUAL ((integer_sequence<char, 2>::size()), 1);
        BOOST_REQUIRE_EQUAL ((integer_sequence<int, 2, 1, 4, 5, 0>::size()), 5);
      }

      BOOST_AUTO_TEST_CASE (value_type)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (integer_sequence<int>::value_type, int);
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (integer_sequence<std::size_t>::value_type, std::size_t);
      }

      BOOST_AUTO_TEST_CASE (make_sequence)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (make_integer_sequence<int, 0>, integer_sequence<int>);
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (make_integer_sequence<int, 1>, integer_sequence<int, 0>);
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (make_integer_sequence<int, 5>, integer_sequence<int, 0, 1, 2, 3, 4>);
      }

      BOOST_AUTO_TEST_CASE (index_sequence_)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (index_sequence<>, integer_sequence<std::size_t>);
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (index_sequence<3>, integer_sequence<std::size_t, 3>);

        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (make_index_sequence<0>, index_sequence<>);
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (make_index_sequence<1>, index_sequence<0>);

        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (index_sequence_for<>, index_sequence<>);
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (index_sequence_for<int, float, char>, index_sequence<0, 1, 2>);
      }
    }
  }
}

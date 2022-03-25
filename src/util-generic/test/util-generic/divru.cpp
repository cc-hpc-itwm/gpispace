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

#include <boost/test/unit_test.hpp>

#include <util-generic/divru.hpp>
#include <util-generic/hard_integral_typedef.hpp>
#include <util-generic/testing/printer/hard_integral_typedef.hpp>
#include <util-generic/testing/random.hpp>

//! \todo use BOOST_DATA_TEST_CASE
BOOST_AUTO_TEST_CASE (zero)
{
  BOOST_REQUIRE_EQUAL (fhg::util::divru (0, 1), 0);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (0, 10000), 0);
}

BOOST_AUTO_TEST_CASE (minmax)
{
  BOOST_REQUIRE_EQUAL (fhg::util::divru (1, 1), 1);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (100000, 1), 100000);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (100000, 100000), 1);
}

BOOST_AUTO_TEST_CASE (div2)
{
  BOOST_REQUIRE_EQUAL (fhg::util::divru (0, 2), 0);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (1, 2), 1);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (2, 2), 1);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (3, 2), 2);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (4, 2), 2);
}

BOOST_AUTO_TEST_CASE (negative)
{
  BOOST_REQUIRE_EQUAL (fhg::util::divru (-4, 2), -1);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (-3, 2), -1);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (-2, 2), 0);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (-1, 2), 0);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (0, 2), 0);

}

BOOST_AUTO_TEST_CASE (a_bit_of_random)
{
  BOOST_REQUIRE_EQUAL (fhg::util::divru (9, 2), 5);

  BOOST_REQUIRE_EQUAL (fhg::util::divru (98731873, 210242), 470);

  static_assert ( sizeof (unsigned long) > sizeof (unsigned int)
                , "note: int -> long to avoid issue of overflow"
                );
  unsigned long const d
    ( fhg::util::testing::random<unsigned int>{}
        (fhg::util::testing::random<unsigned int>::non_zero{})
    );

  BOOST_REQUIRE_EQUAL (fhg::util::divru (0ul, d), 0ul);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (1ul, d), 1ul);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (d - 1ul, d), 1ul);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (d, d), 1ul);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (2ul * d, d), 2ul);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (2ul * d + 1ul, d), 3ul);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (3ul * d - 1ul, d), 3ul);
  BOOST_REQUIRE_EQUAL (fhg::util::divru (3ul * d, d), 3ul);
}

namespace fhg
{
  namespace util
  {
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (test_t, std::size_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (+, test_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (-, test_t);
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF_ARITH_OPERATOR (/, test_t);
  }
}

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_LOG_VALUE_PRINTER (fhg::util::test_t)

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (with_hard_integral_typedef)
    {
      BOOST_REQUIRE_EQUAL (divru (test_t {0}, test_t {1}), test_t {0});
      BOOST_REQUIRE_EQUAL (divru (test_t {1}, test_t {1}), test_t {1});
      BOOST_REQUIRE_EQUAL
        (divru (test_t {98731873}, test_t {210242}), test_t {470});
    }
  }
}

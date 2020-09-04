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

#include <util-generic/testing/require_compiletime.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      BOOST_AUTO_TEST_CASE (true_condition_succeeds)
      {
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE (true);
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE (!false);

        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_EQUAL (0, 0);
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_NE (1, 0);
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_LT (0, 1);
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_LE (0, 0);
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_GT (1, 0);
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_GE (0, 0);
      }

      BOOST_AUTO_TEST_CASE
        (false_condition_fails, *boost::unit_test::expected_failures (8))
      {
        FHG_UTIL_TESTING_COMPILETIME_CHECK (!true);
        FHG_UTIL_TESTING_COMPILETIME_CHECK (false);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL (1, 0);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_NE (0, 0);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_LT (0, 0);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_LE (1, 0);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_GT (0, 0);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_GE (0, 1);
      }

      BOOST_AUTO_TEST_CASE (condition_can_contain_commas)
      {
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE (std::is_same<void, void>{});

        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_EQUAL (std::integral_constant<int, 0>{}, std::integral_constant<int, 0>{});
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_NE (std::integral_constant<int, 1>{}, std::integral_constant<int, 0>{});
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_LT (std::integral_constant<int, 0>{}, std::integral_constant<int, 1>{});
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_LE (std::integral_constant<int, 0>{}, std::integral_constant<int, 0>{});
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_GT (std::integral_constant<int, 1>{}, std::integral_constant<int, 0>{});
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE_GE (std::integral_constant<int, 0>{}, std::integral_constant<int, 0>{});
      }

//! \todo require compile failure if macro not defined and success when defined
/*
#ifdef FHG_UTIL_TESTING_TEST_CONDITION_IS_KNOWN_AT_COMPILE_TIME
      constexpr bool const condition (true);
#else
      extern bool condition;
#endif

      BOOST_AUTO_TEST_CASE (condition_needs_to_be_a_compile_time_constant)
      {
        FHG_UTIL_TESTING_COMPILETIME_REQUIRE (condition);
      }
*/
    }
  }
}

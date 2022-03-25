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

#include <util-generic/in_this_scope.hpp>

#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (in_this_scope_sets_and_restores_value)
    {
      testing::unique_random<int> unique_random_int;

      auto const value (unique_random_int());
      auto const other_value (unique_random_int());

      int variable = value;

      {
        FHG_UTIL_IN_THIS_SCOPE (variable) = other_value;

        BOOST_REQUIRE_EQUAL (variable, other_value);
      }

      BOOST_REQUIRE_EQUAL (variable, value);
    }

    BOOST_AUTO_TEST_CASE
      (in_this_scope_works_correct_when_scope_is_left_via_an_exception)
    {
      testing::unique_random<int> unique_random_int;

      auto const value (unique_random_int());
      auto const other_value (unique_random_int());

      int variable = value;

      try
      {
        FHG_UTIL_IN_THIS_SCOPE (variable) = other_value;

        throw unique_random_int();
      }
      catch (...)
      {
        // ignore
      }

      BOOST_REQUIRE_EQUAL (variable, value);
    }

    BOOST_AUTO_TEST_CASE (in_this_scope_can_be_done_multiple_times)
    {
      testing::unique_random<int> unique_random_int;

      auto const value (unique_random_int());
      auto const other_value (unique_random_int());
      auto const and_another_value (unique_random_int());

      int variable = value;

      {
        FHG_UTIL_IN_THIS_SCOPE (variable) = other_value;

        BOOST_REQUIRE_EQUAL (variable, other_value);

        FHG_UTIL_IN_THIS_SCOPE (variable) = and_another_value;

        BOOST_REQUIRE_EQUAL (variable, and_another_value);
      }

      BOOST_REQUIRE_EQUAL (variable, value);
    }

    BOOST_AUTO_TEST_CASE (in_this_scope_can_be_nested)
    {
      testing::unique_random<int> unique_random_int;

      auto const value (unique_random_int());
      auto const other_value (unique_random_int());
      auto const and_another_value (unique_random_int());

      int variable = value;

      {
        FHG_UTIL_IN_THIS_SCOPE (variable) = other_value;

        {
          FHG_UTIL_IN_THIS_SCOPE (variable) = and_another_value;

          BOOST_REQUIRE_EQUAL (variable, and_another_value);
        }

        BOOST_REQUIRE_EQUAL (variable, other_value);
      }

      BOOST_REQUIRE_EQUAL (variable, value);
    }

    namespace
    {
      struct A
      {
      };
      struct B
      {
        operator A() const
        {
          return {};
        }
      };
      struct C
      {
        C() = default;
        C (A) {}
      };
    }

    BOOST_AUTO_TEST_CASE (in_this_scope_forwards_arguments_to_constructor)
    {
      C c;

      FHG_UTIL_IN_THIS_SCOPE (c) = B{};
    }

    namespace
    {
      struct S
      {
        S() : value (0) {}
        S (int x, int y) : value (x + y) {}

        int value;
      };
    }

    BOOST_AUTO_TEST_CASE
      (in_this_scope_has_alternative_syntax_to_delay_construction)
    {
      testing::random<int> random_int;

      auto const x (random_int());
      auto const y (random_int());

      S s;

      {
        FHG_UTIL_IN_THIS_SCOPE (s) (x, y);

        BOOST_REQUIRE_EQUAL (s.value, x + y);
      }

      BOOST_REQUIRE_EQUAL (s.value, 0);

      {
        FHG_UTIL_IN_THIS_SCOPE (s) = S (x, y);

        BOOST_REQUIRE_EQUAL (s.value, x + y);
      }

      BOOST_REQUIRE_EQUAL (s.value, 0);
    }
  }
}

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

#include <util-generic/temporary_return_value_holder.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    namespace
    {
      struct checker
      {
        int currently_expected_i;
        std::size_t currently_expected_j;
        checker (int i, std::size_t j)
          : currently_expected_i (i)
          , currently_expected_j (j)
        {}

        std::size_t was_called = 0;

        void operator() (int i, std::size_t j)
        {
          BOOST_REQUIRE_EQUAL (currently_expected_i, i);
          BOOST_REQUIRE_EQUAL (currently_expected_j, j);

          ++was_called;
        }

        std::string operator() (int i)
        {
          BOOST_REQUIRE_EQUAL (currently_expected_i, i);

          ++was_called;

          return std::to_string (i);
        }
      };
    }

    BOOST_AUTO_TEST_CASE (calls_function_with_given_arguments)
    {
      auto const i (testing::random<int>{}());
      auto const j (testing::random<std::size_t>{}());

      checker checker {i, j};

      temporary_return_value_holder<void> {checker, i, j};
      temporary_return_value_holder<std::string> {checker, i};

      BOOST_REQUIRE_EQUAL (checker.was_called, 2);
    }

    BOOST_AUTO_TEST_CASE (holds_return_value_if_non_void)
    {
      auto const i (testing::random<int>{}());
      auto const j (testing::random<std::size_t>{}());

      checker checker {i, j};

      temporary_return_value_holder<void> voider {checker, i, j};
      temporary_return_value_holder<std::string> stringer {checker, i};

      FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL (decltype (*voider), void);
      FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL (decltype (*stringer), std::string);

      *voider;
      BOOST_REQUIRE_EQUAL (*stringer, std::to_string (i));
    }

    BOOST_AUTO_TEST_CASE (works_with_references)
    {
      int i (0);

      auto const get_ref ([&] () -> int& { return i; });

      temporary_return_value_holder<int&> tv {get_ref};

      ++*tv;

      BOOST_REQUIRE_EQUAL (i, 1);
    }
  }
}

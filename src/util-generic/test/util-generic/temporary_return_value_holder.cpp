// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

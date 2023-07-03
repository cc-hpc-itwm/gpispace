// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/cxx17/logical_operator_type_traits.hpp>
#include <util-generic/testing/require_compiletime.hpp>
#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      BOOST_AUTO_TEST_CASE (conjunction_is_true_if_all_are_true)
      {
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL (conjunction<>{}, true);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (conjunction<std::true_type>{}, true);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (conjunction<std::false_type>{}, false);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (conjunction<std::true_type, std::true_type>{}, true);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (conjunction<std::true_type, std::false_type>{}, false);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (conjunction<std::false_type, std::true_type>{}, false);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (conjunction<std::false_type, std::false_type>{}, false);
      }

      BOOST_AUTO_TEST_CASE (disjunction_is_true_if_any_is_true)
      {
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL (disjunction<>{}, false);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (disjunction<std::true_type>{}, true);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (disjunction<std::false_type>{}, false);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (disjunction<std::true_type, std::true_type>{}, true);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (disjunction<std::true_type, std::false_type>{}, true);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (disjunction<std::false_type, std::true_type>{}, true);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (disjunction<std::false_type, std::false_type>{}, false);
      }

      BOOST_AUTO_TEST_CASE (negation_is_true_if_false)
      {
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (negation<std::true_type>{}, false);
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (negation<std::false_type>{}, true);
      }

      BOOST_AUTO_TEST_CASE
        (disjunction_returns_first_evaluated_to_true_or_last_to_false)
      {
        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
          ( disjunction < std::false_type
                        , std::integral_constant<char, 0>
                        , std::integral_constant<int, 125>
                        , std::integral_constant<char, 52>
                        >::type
          , std::integral_constant<int, 125>
          );

        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
          ( disjunction < std::false_type
                        , std::integral_constant<char, 0>
                        , std::integral_constant<int, 0>
                        >::type
          , std::integral_constant<int, 0>
          );
      }

      BOOST_AUTO_TEST_CASE
        (conjunction_returns_last_evaluating_to_true_or_first_false)
      {
        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
          ( conjunction < std::integral_constant<bool, true>
                        , std::integral_constant<char, 52>
                        , std::true_type
                        , std::integral_constant<int, 125>
                        >::type
          , std::integral_constant<int, 125>
          );

        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
          ( conjunction < std::integral_constant<bool, true>
                        , std::integral_constant<char, 0>
                        , std::true_type
                        , std::integral_constant<int, 125>
                        >::type
          , std::integral_constant<char, 0>
          );
      }

      BOOST_AUTO_TEST_CASE (negation_narrows_to_bool)
      {
        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
          (negation<std::integral_constant<char, 52>>::type, std::false_type);

        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
          (negation<std::integral_constant<unsigned, 0>>::type, std::true_type);
      }

      BOOST_AUTO_TEST_CASE (xjunction_short_circuit)
      {
        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (disjunction<std::true_type, void>{}, true);

        FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL
          (conjunction<std::false_type, void>{}, false);
      }
    }
  }
}

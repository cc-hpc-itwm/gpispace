// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/mp/apply.hpp>
#include <util-generic/mp/exactly_one_is.hpp>
#include <util-generic/mp/find.hpp>
#include <util-generic/mp/none_is.hpp>
#include <util-generic/mp/rebind.hpp>
#include <util-generic/mp/remove_duplicates.hpp>
#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace mp
    {
      BOOST_AUTO_TEST_CASE (none_is_missing)
      {
        BOOST_REQUIRE ((none_is<int>::value));
        BOOST_REQUIRE ((none_is<int, char>::value));
        BOOST_REQUIRE ((none_is<int, float, char>::value));
      }
      BOOST_AUTO_TEST_CASE (none_is_present)
      {
        BOOST_REQUIRE ((!none_is<int, int>::value));
        BOOST_REQUIRE ((!none_is<int, float, int>::value));
        BOOST_REQUIRE ((!none_is<int, int, float>::value));
      }

      BOOST_AUTO_TEST_CASE (exactly_one_is_one_present)
      {
        BOOST_REQUIRE ((exactly_one_is<int, int>::value));
        BOOST_REQUIRE ((exactly_one_is<int, int, char>::value));
        BOOST_REQUIRE ((exactly_one_is<int, char, int>::value));
      }
      BOOST_AUTO_TEST_CASE (exactly_one_is_none_present)
      {
        BOOST_REQUIRE ((!exactly_one_is<int>::value));
        BOOST_REQUIRE ((!exactly_one_is<int, char>::value));
      }
      BOOST_AUTO_TEST_CASE (exactly_one_is_multiple_present)
      {
        BOOST_REQUIRE ((!exactly_one_is<int, int, int>::value));
      }

      BOOST_AUTO_TEST_CASE (find_finds)
      {
        BOOST_REQUIRE_EQUAL ((find<int, int>::value), 0);
        BOOST_REQUIRE_EQUAL ((find<int, char, int>::value), 1);
        BOOST_REQUIRE_EQUAL ((find<int, char, int, float>::value), 1);
        BOOST_REQUIRE_EQUAL ((find<int, char, float, int>::value), 2);
      }
      BOOST_AUTO_TEST_CASE (find_returns_first_index)
      {
        BOOST_REQUIRE_EQUAL ((find<int, int, int>::value), 0);
        BOOST_REQUIRE_EQUAL ((find<int, char, int, int>::value), 1);
      }

      namespace
      {
        template<typename...> struct sequence {};
        template<typename...> struct sequence2 {};
        template<typename...> struct sequence3 {};
      }

      BOOST_AUTO_TEST_CASE (rebind_empty)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (rebind<sequence2, sequence<>>, sequence2<>);
      }

      BOOST_AUTO_TEST_CASE (rebind_back_and_forth)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (rebind<sequence, rebind<sequence2, sequence<>>>, sequence<>);
      }

      BOOST_AUTO_TEST_CASE (rebind_with_entries)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (rebind<sequence2, sequence<int>>, sequence2<int>);
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (rebind<sequence2, sequence<int, float>>, sequence2<int, float>);
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (rebind<sequence, rebind<sequence2, sequence<int>>>, sequence<int>);
      }

      BOOST_AUTO_TEST_CASE (rebind_only_touches_outermost)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( rebind<sequence2, sequence<sequence<int>>>
          , sequence2<sequence<int>>
          );
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( rebind<sequence2, sequence<sequence3<int>>>
          , sequence2<sequence3<int>>
          );
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( rebind<sequence2, sequence3<sequence<int>>>
          , sequence2<sequence<int>>
          );
      }

      BOOST_AUTO_TEST_CASE (remove_duplicates_empty_sequence)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( remove_duplicates<sequence<>>
          , sequence<>
          );
      }

      BOOST_AUTO_TEST_CASE (remove_duplicates_unique_values)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( remove_duplicates<sequence<int, char>>
          , sequence<int, char>
          );
      }

      BOOST_AUTO_TEST_CASE (remove_duplicates_ints)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( remove_duplicates<sequence<int, int, int>>
          , sequence<int>
          );
      }

      namespace
      {
        template<typename = void, typename = void, typename = void>
          struct sequence_with_defaults {};
      }

      BOOST_AUTO_TEST_CASE (remove_duplicates_with_sequence_with_defaults)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( remove_duplicates<sequence_with_defaults<int, float>>
          , sequence_with_defaults<int, float>
          );
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( remove_duplicates<sequence_with_defaults<int, int>>
          , sequence_with_defaults<int>
          );
      }

      BOOST_AUTO_TEST_CASE (remove_duplicates_does_not_require_consequtive)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( remove_duplicates<sequence<int, char, int, char, int>>
          , sequence<int, char>
          );
      }

      namespace
      {
        template<typename>
          using to_void = void;
        template<typename T>
          using filter_int
            = typename std::conditional<std::is_same<T, int>::value, void,  T>::type;
      }

      BOOST_AUTO_TEST_CASE (applied)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( apply<to_void, sequence<int, char>>
          , sequence<void, void>
          );

        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( apply<filter_int, sequence<int, char, int>>
          , sequence<void, char, void>
          );
      }
    }
  }
}

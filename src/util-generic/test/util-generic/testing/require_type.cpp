// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      extern char const* some_function (int);

      BOOST_AUTO_TEST_CASE (equal_test_succeeds_if_matching_types)
      {
        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (int, int);
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL (int, int);

        using a_struct = struct { int a; };
        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (a_struct, a_struct);
        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (std::string, std::string);
        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
          ( equal_test_succeeds_if_matching_types
          , equal_test_succeeds_if_matching_types
          );
        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
          (decltype (&some_function), decltype (&some_function));
        using a_typedef = int;
        FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (a_typedef, int);
      }

      BOOST_AUTO_TEST_CASE (ne_test_succeeds_if_mismatching_types)
      {
        FHG_UTIL_TESTING_CHECK_TYPE_NE (int, float);
        FHG_UTIL_TESTING_REQUIRE_TYPE_NE (int, float);

        using a_struct = struct { int a; };
        using b_struct = struct { int b; };
        FHG_UTIL_TESTING_CHECK_TYPE_NE (a_struct, b_struct);
        FHG_UTIL_TESTING_CHECK_TYPE_NE (std::string, std::size_t);
        FHG_UTIL_TESTING_CHECK_TYPE_NE
          ( equal_test_succeeds_if_matching_types
          , ne_test_succeeds_if_mismatching_types
          );
        FHG_UTIL_TESTING_CHECK_TYPE_NE
          (decltype (&some_function), std::string (int));
        using a_typedef = int;
        FHG_UTIL_TESTING_CHECK_TYPE_NE (a_typedef, float);
      }

#ifdef FHG_UTIL_TESTING_HAVE_CXXABI
      BOOST_AUTO_TEST_CASE (types_are_pretty_printed)
      {
        {
          std::ostringstream oss;
          detail::show_pretty_typename<int> (oss);
          BOOST_REQUIRE_EQUAL (oss.str(), "int");
        }

        {
          std::ostringstream oss;
          detail::show_pretty_typename<int (*)(float)> (oss);
          BOOST_REQUIRE_EQUAL (oss.str(), "int (*)(float)");
        }
      }
#endif
    }
  }
}

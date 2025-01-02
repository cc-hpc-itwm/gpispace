// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/cxx17/void_t.hpp>
#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      BOOST_AUTO_TEST_CASE (empty)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL (void_t<>, void);
      }

      BOOST_AUTO_TEST_CASE (void_)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL (void_t<void>, void);
      }

      BOOST_AUTO_TEST_CASE (void_void_void_void_void_void)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (void_t<void, void, void, void, void, void>, void);
      }

      BOOST_AUTO_TEST_CASE (any_type)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          (void_t<int, float, std::string, void_t<int>>, void);
      }
    }
  }
}

// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/require_container_is_permutation.hpp>

#include <util-generic/testing/printer/multimap.hpp>
#include <util-generic/testing/printer/vector.hpp>

#include <boost/test/unit_test.hpp>

#include <map>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      BOOST_AUTO_TEST_CASE (different_size_is_noticed)
      {
        std::vector<int> const a {0, 1, 2, 2};
        std::vector<int> const b {0, 1, 2};

        FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (a, b);
      }

      BOOST_AUTO_TEST_CASE (different_content_is_noticed)
      {
        std::multimap<int, float> const a {{0, 1.f}, {0, 2.f}, {1, 3.f}};
        std::multimap<int, float> const b {{0, 2.f}, {0, 1.f}, {1, 3.1f}};

        FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION (a, b);
      }
    }
  }
}

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

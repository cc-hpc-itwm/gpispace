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

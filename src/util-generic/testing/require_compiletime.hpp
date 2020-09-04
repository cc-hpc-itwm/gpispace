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

#pragma once

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      //! Checks that the given condition_ can be compile-time
      //! evaluated but generates a runtime error message if
      //! condition_ evaluates to false.
#define FHG_UTIL_TESTING_COMPILETIME_REQUIRE(condition_...)             \
      FHG_UTIL_TESTING_COMPILETIME_CHECK_IMPL (REQUIRE, condition_)
#define FHG_UTIL_TESTING_COMPILETIME_CHECK(condition_...)               \
      FHG_UTIL_TESTING_COMPILETIME_CHECK_IMPL (CHECK, condition_)

      //! Checks that lhs_ == rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define FHG_UTIL_TESTING_COMPILETIME_REQUIRE_EQUAL(lhs_, rhs_...)       \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_equal, lhs_, rhs_)
#define FHG_UTIL_TESTING_COMPILETIME_CHECK_EQUAL(lhs_, rhs_...)         \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_equal, lhs_, rhs_)

      //! Checks that lhs_ != rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define FHG_UTIL_TESTING_COMPILETIME_REQUIRE_NE(lhs_, rhs_...)          \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_not_equal, lhs_, rhs_)
#define FHG_UTIL_TESTING_COMPILETIME_CHECK_NE(lhs_, rhs_...)            \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_not_equal, lhs_, rhs_)

      //! Checks that lhs_ < rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define FHG_UTIL_TESTING_COMPILETIME_REQUIRE_LT(lhs_, rhs_...)          \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_less, lhs_, rhs_)
#define FHG_UTIL_TESTING_COMPILETIME_CHECK_LT(lhs_, rhs_...)            \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_less, lhs_, rhs_)

      //! Checks that lhs_ <= rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define FHG_UTIL_TESTING_COMPILETIME_REQUIRE_LE(lhs_, rhs_...)          \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_less_equal, lhs_, rhs_)
#define FHG_UTIL_TESTING_COMPILETIME_CHECK_LE(lhs_, rhs_...)            \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_less_equal, lhs_, rhs_)

      //! Checks that lhs_ > rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define FHG_UTIL_TESTING_COMPILETIME_REQUIRE_GT(lhs_, rhs_...)          \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_greater, lhs_, rhs_)
#define FHG_UTIL_TESTING_COMPILETIME_CHECK_GT(lhs_, rhs_...)            \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_greater, lhs_, rhs_)

      //! Checks that lhs_ >= rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define FHG_UTIL_TESTING_COMPILETIME_REQUIRE_GE(lhs_, rhs_...)          \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_greater_equal, lhs_, rhs_)
#define FHG_UTIL_TESTING_COMPILETIME_CHECK_GE(lhs_, rhs_...)            \
      FHG_UTIL_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_greater_equal, lhs_, rhs_)
    }
  }
}

#include <util-generic/testing/require_compiletime.ipp>

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

#include <util-generic/testing/printer/generic.hpp>

#include <boost/test/tools/context.hpp>
#include <boost/test/tools/interface.hpp>

#include <algorithm>

#define FHG_UTIL_TESTING_REQUIRE_CONTAINER_IS_PERMUTATION_IMPL(lhs_, rhs_)     \
  do                                                                           \
  {                                                                            \
    /* \note Doesn't use a function for new scope to keep source location */   \
    /* in the error message, but should also to allow for expressions as */    \
    /* arguments, so requires a temporary. lhs and rhs are quite common, so */ \
    /* so prefix them with the macro name. */                                  \
    auto const futrcipi_lhs (lhs_);                                            \
    auto const futrcipi_rhs (rhs_);                                            \
    BOOST_TEST_CONTEXT                                                         \
      (#lhs_ " = " << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (futrcipi_lhs))    \
    BOOST_TEST_CONTEXT                                                         \
      (#rhs_ " = " << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (futrcipi_rhs))    \
    {                                                                          \
      BOOST_REQUIRE_EQUAL (futrcipi_lhs.size(), futrcipi_rhs.size());          \
      BOOST_REQUIRE_MESSAGE                                                    \
        ( std::is_permutation                                                  \
            (futrcipi_lhs.begin(), futrcipi_lhs.end(), futrcipi_rhs.begin())   \
        , "the elements shall be a permutation of the expected value"          \
        );                                                                     \
    }                                                                          \
  }                                                                            \
  while (false)

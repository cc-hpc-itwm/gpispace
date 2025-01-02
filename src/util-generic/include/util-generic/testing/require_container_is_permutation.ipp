// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

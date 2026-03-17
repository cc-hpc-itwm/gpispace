// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once



    namespace gspc::testing
    {
      //! Checks that the given condition_ can be compile-time
      //! evaluated but generates a runtime error message if
      //! condition_ evaluates to false.
#define GSPC_TESTING_COMPILETIME_REQUIRE(condition_...)             \
      GSPC_TESTING_COMPILETIME_CHECK_IMPL (REQUIRE, condition_)
#define GSPC_TESTING_COMPILETIME_CHECK(condition_...)               \
      GSPC_TESTING_COMPILETIME_CHECK_IMPL (CHECK, condition_)

      //! Checks that lhs_ == rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define GSPC_TESTING_COMPILETIME_REQUIRE_EQUAL(lhs_, rhs_...)       \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_equal, lhs_, rhs_)
#define GSPC_TESTING_COMPILETIME_CHECK_EQUAL(lhs_, rhs_...)         \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_equal, lhs_, rhs_)

      //! Checks that lhs_ != rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define GSPC_TESTING_COMPILETIME_REQUIRE_NE(lhs_, rhs_...)          \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_not_equal, lhs_, rhs_)
#define GSPC_TESTING_COMPILETIME_CHECK_NE(lhs_, rhs_...)            \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_not_equal, lhs_, rhs_)

      //! Checks that lhs_ < rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define GSPC_TESTING_COMPILETIME_REQUIRE_LT(lhs_, rhs_...)          \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_less, lhs_, rhs_)
#define GSPC_TESTING_COMPILETIME_CHECK_LT(lhs_, rhs_...)            \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_less, lhs_, rhs_)

      //! Checks that lhs_ <= rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define GSPC_TESTING_COMPILETIME_REQUIRE_LE(lhs_, rhs_...)          \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_less_equal, lhs_, rhs_)
#define GSPC_TESTING_COMPILETIME_CHECK_LE(lhs_, rhs_...)            \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_less_equal, lhs_, rhs_)

      //! Checks that lhs_ > rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define GSPC_TESTING_COMPILETIME_REQUIRE_GT(lhs_, rhs_...)          \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_greater, lhs_, rhs_)
#define GSPC_TESTING_COMPILETIME_CHECK_GT(lhs_, rhs_...)            \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_greater, lhs_, rhs_)

      //! Checks that lhs_ >= rhs_ can be compile-time evaluated but
      //! generates a runtime error message if the comparison fails.
#define GSPC_TESTING_COMPILETIME_REQUIRE_GE(lhs_, rhs_...)          \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (REQUIRE, is_greater_equal, lhs_, rhs_)
#define GSPC_TESTING_COMPILETIME_CHECK_GE(lhs_, rhs_...)            \
      GSPC_TESTING_COMPILETIME_RELATION_IMPL (CHECK, is_greater_equal, lhs_, rhs_)
    }



#include <gspc/testing/require_compiletime.ipp>

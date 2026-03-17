// Copyright (C) 2012,2014-2016,2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/assert.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/require_exception.hpp>

#include <fmt/core.h>

BOOST_AUTO_TEST_CASE (assert_true)
{
  gspc_assert(1 == 1, "assert_true test case");
}

BOOST_AUTO_TEST_CASE (assert_false)
{
  gspc::testing::require_exception
    ( []() { gspc_assert (1 == 0, "util_assert_false"); }
    , std::logic_error
        { fmt::format
          ( "[{}:{}] assertion '{}' failed{}{}."
          , __FILE__
          , __LINE__ - 5
          , "1 == 0"
          , ": "
          , "util_assert_false"
          )
        }
    );
}

BOOST_AUTO_TEST_CASE (assert_true_empty_message)
{
  gspc_assert(1 == 1);
}

BOOST_AUTO_TEST_CASE (assert_false_empty_message)
{
  gspc::testing::require_exception
    ( []() { gspc_assert (1 == 0); }
    , std::logic_error
        { fmt::format
          ( "[{}:{}] assertion '{}' failed{}{}."
          , __FILE__
          , __LINE__ - 5
          , "1 == 0"
          , ""
          , ""
          )
        }
    );
}

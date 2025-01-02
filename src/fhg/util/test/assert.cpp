// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <fhg/assert.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <fmt/core.h>

BOOST_AUTO_TEST_CASE (assert_true)
{
  fhg_assert(1 == 1, "assert_true test case");
}

BOOST_AUTO_TEST_CASE (assert_false)
{
  fhg::util::testing::require_exception
    ( []() { fhg_assert (1 == 0, "util_assert_false"); }
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
  fhg_assert(1 == 1);
}

BOOST_AUTO_TEST_CASE (assert_false_empty_message)
{
  fhg::util::testing::require_exception
    ( []() { fhg_assert (1 == 0); }
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

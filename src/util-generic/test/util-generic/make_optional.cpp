// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/make_optional.hpp>

#include <boost/optional/optional_io.hpp>

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (how_is_never_evaluated_on_false_cond)
    {
      std::size_t evals (0);
      FHG_UTIL_MAKE_OPTIONAL (false, ++evals);

      BOOST_REQUIRE_EQUAL (evals, 0);
    }

    BOOST_AUTO_TEST_CASE (how_is_evaluated_once_on_true_cond)
    {
      std::size_t evals (0);
      FHG_UTIL_MAKE_OPTIONAL (true, ++evals);

      BOOST_REQUIRE_EQUAL (evals, 1);
    }

    BOOST_AUTO_TEST_CASE (value_of_optional_is_none_on_false_cond)
    {
      BOOST_REQUIRE_EQUAL (!!FHG_UTIL_MAKE_OPTIONAL (false, true), false);
    }

    BOOST_AUTO_TEST_CASE (value_of_optional_is_the_one_given_on_true_cond)
    {
      BOOST_REQUIRE_EQUAL (!!FHG_UTIL_MAKE_OPTIONAL (true, false), true);
      BOOST_REQUIRE_EQUAL (FHG_UTIL_MAKE_OPTIONAL (true, false), false);
    }
  }
}

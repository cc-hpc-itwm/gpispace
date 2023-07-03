// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/getenv.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (non_existing_returns_none)
    {
      BOOST_REQUIRE_EQUAL (getenv (""), ::boost::none);
      BOOST_REQUIRE_EQUAL
        (getenv ("this/hopefully(( does not exist"), ::boost::none);
    }

    BOOST_AUTO_TEST_CASE (returns_correct_value_if_set)
    {
      auto const value ( testing::random<std::string>{}
                           (testing::random<char>::any_without_zero())
                       );
      auto const key ("_fhg_util_getenv_test_variable_");

      syscall::setenv (key, value.c_str(), true);

      BOOST_REQUIRE_EQUAL (getenv (key).get(), value);
    }
  }
}

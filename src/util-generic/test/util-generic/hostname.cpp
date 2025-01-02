// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/hostname.hpp>
#include <util-generic/syscall.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace
    {
      auto const env_HOSTNAME (syscall::getenv ("HOSTNAME"));

      ::boost::test_tools::assertion_result env_HOSTNAME_set
        (::boost::unit_test::test_unit_id)
      {
        ::boost::test_tools::assertion_result result (!!env_HOSTNAME);
        result.message() << "environment variable HOSTNAME is not set";
        return result;
      }
    }

    BOOST_AUTO_TEST_CASE ( is_equal_to_environment
                         , *::boost::unit_test::precondition (env_HOSTNAME_set)
                         )
    {
      BOOST_REQUIRE_EQUAL (hostname(), env_HOSTNAME);
    }

    BOOST_AUTO_TEST_CASE ( works_without_environment
                         , *::boost::unit_test::precondition (env_HOSTNAME_set)
                         )
    {
      syscall::unsetenv ("HOSTNAME");
      BOOST_REQUIRE_EQUAL (hostname(), env_HOSTNAME);
    }
  }
}

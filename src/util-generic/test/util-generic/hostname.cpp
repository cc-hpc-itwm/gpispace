// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

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
      BOOST_REQUIRE_EQUAL (getenv (""), boost::none);
      BOOST_REQUIRE_EQUAL
        (getenv ("this/hopefully(( does not exist"), boost::none);
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

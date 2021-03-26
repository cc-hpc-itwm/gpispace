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

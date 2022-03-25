// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-generic/first_then.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

BOOST_AUTO_TEST_CASE (first_then_works)
{
  std::string const first {fhg::util::testing::random<std::string>{}()};
  std::string const then {fhg::util::testing::random<std::string>{}()};

  fhg::util::first_then<std::string> const first_then {first, then};

  BOOST_REQUIRE_EQUAL (first_then.string(), first);
  BOOST_REQUIRE_EQUAL (first_then.string(), then);
  BOOST_REQUIRE_EQUAL (first_then.string(), then);
}

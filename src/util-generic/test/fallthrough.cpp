// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/fallthrough.hpp>

BOOST_AUTO_TEST_CASE (fallthrough_compiles_and_falls_through)
{
  auto const check ( [] (int x) -> int
                     {
                       switch (x)
                       {
                       case 1: ++x; FHG_UTIL_FALLTHROUGH;
                       case 2: ++x; break;
                       default: x = -1;
                       }

                       return x;
                     }
                   );

  for (int x (-1000); x < 1; ++x)
  {
    BOOST_REQUIRE (check (x) == -1);
  }

  BOOST_REQUIRE (check (1) == 3);
  BOOST_REQUIRE (check (2) == 3);

  for (int x (3); x < 1000; ++x)
  {
    BOOST_REQUIRE (check (x) == -1);
  }
}

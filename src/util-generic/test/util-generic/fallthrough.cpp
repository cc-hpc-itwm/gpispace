// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

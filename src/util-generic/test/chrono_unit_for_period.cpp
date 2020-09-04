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

#include <util-generic/chrono_unit_for_period.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    BOOST_AUTO_TEST_CASE (instances_for_predefined_units_are_defined)
    {
#define CHRONO_UNIT_FOR_PREDEFINED_PERIOD(symbol_, ratio_)              \
      BOOST_REQUIRE_EQUAL (symbol_, chrono_unit_for_period<ratio_::period>())

      CHRONO_UNIT_FOR_PREDEFINED_PERIOD ("ns", std::chrono::nanoseconds);
      CHRONO_UNIT_FOR_PREDEFINED_PERIOD ("µs", std::chrono::microseconds);
      CHRONO_UNIT_FOR_PREDEFINED_PERIOD ("ms", std::chrono::milliseconds);
      CHRONO_UNIT_FOR_PREDEFINED_PERIOD ("s", std::chrono::seconds);
      CHRONO_UNIT_FOR_PREDEFINED_PERIOD ("min", std::chrono::minutes);
      CHRONO_UNIT_FOR_PREDEFINED_PERIOD ("hr", std::chrono::hours);

#undef CHRONO_UNIT_FOR_PREDEFINED_PERIOD
    }

    BOOST_AUTO_TEST_CASE (den_one_omits_slash)
    {
      BOOST_REQUIRE_EQUAL ("[3]s", (chrono_unit_for_period<std::ratio<3>>()));
      BOOST_REQUIRE_EQUAL ("[4]s", (chrono_unit_for_period<std::ratio<4>>()));
      BOOST_REQUIRE_EQUAL ("[5]s", (chrono_unit_for_period<std::ratio<5>>()));
    }
    BOOST_AUTO_TEST_CASE (arbitrary_ranges_are_just_displayed)
    {
      BOOST_REQUIRE_EQUAL ( "[3/5]s"
                          , (chrono_unit_for_period<std::ratio<3, 5>>())
                          );
      BOOST_REQUIRE_EQUAL ( "[7/9]s"
                          , (chrono_unit_for_period<std::ratio<7, 9>>())
                          );
    }
  }
}

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

#include <sdpa/requirements_and_preferences.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <fhg/util/next.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (only_different_preferences_are_allowed)
{
  using fhg::util::testing::random;
  using fhg::util::testing::unique_randoms;

  unsigned int const MAX_PREFERENCES (100);
  unsigned int const MIN_PREFERENCES (1);

  auto preferences
    ( unique_randoms<Preferences>
        (random<std::size_t>{} (MAX_PREFERENCES, MIN_PREFERENCES))
    );

  auto const insertion_point
    (random<std::size_t>{} (preferences.size() - 1));
  preferences.emplace
    (fhg::util::next (preferences.begin(), insertion_point), preferences.front());

  fhg::util::testing::require_exception
    ( [&]
      {
        Requirements_and_preferences
          ({}, {}, {}, {}, {}, preferences);
      }
    , std::runtime_error ("the preferences must be distinct!")
    );
}

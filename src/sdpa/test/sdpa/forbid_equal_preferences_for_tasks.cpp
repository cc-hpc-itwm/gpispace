// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

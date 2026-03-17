// Copyright (C) 2019-2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/requirements_and_preferences.hpp>

#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <gspc/util/next.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (only_different_preferences_are_allowed)
{
  using gspc::testing::random;
  using gspc::testing::unique_randoms;

  unsigned int const MAX_PREFERENCES (100);
  unsigned int const MIN_PREFERENCES (1);

  auto preferences
    ( unique_randoms<gspc::we::type::Preferences>
        (random<std::size_t>{} (MAX_PREFERENCES, MIN_PREFERENCES))
    );

  auto const insertion_point
    (random<std::size_t>{} (preferences.size() - 1));
  preferences.emplace
    (gspc::util::next (preferences.begin(), insertion_point), preferences.front());

  gspc::testing::require_exception
    ( [&]
      {
        gspc::we::type::Requirements_and_preferences
          ({}, {}, {}, {}, {}, preferences);
      }
    , std::runtime_error ("the preferences must be distinct!")
    );
}

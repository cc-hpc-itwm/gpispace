#include <sdpa/requirements_and_preferences.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <iterator>
#include <stdexcept>

BOOST_AUTO_TEST_CASE (only_different_preferences_are_allowed)
{
  using fhg::util::testing::random_integral;
  using fhg::util::testing::randoms;
  using fhg::util::testing::unique_random;

  unsigned int const MAX_PREFERENCES (100);
  unsigned int const MIN_PREFERENCES (1);

  unsigned int const npreferences
    ( MIN_PREFERENCES
    + random_integral<unsigned int>() % (MAX_PREFERENCES - MIN_PREFERENCES + 1)
    );

  auto preferences (randoms<Preferences, unique_random> (npreferences));

  auto const insertion_point
    (random_integral<std::size_t>() % preferences.size());
  preferences.emplace
    (std::next (preferences.begin(), insertion_point), preferences.front());

  fhg::util::testing::require_exception
    ( [&]
      {
        Requirements_and_preferences
          ({}, {}, {}, {}, {}, preferences);
      }
    , std::runtime_error ("the preferences must be distinct!")
    );
}

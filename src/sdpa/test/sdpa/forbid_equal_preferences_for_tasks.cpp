#include <sdpa/requirements_and_preferences.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>

BOOST_AUTO_TEST_CASE (only_different_preferences_are_allowed)
{
  auto const preference (fhg::util::testing::random<std::string>{}());

  fhg::util::testing::require_exception
    ( [&]
      {
        Requirements_and_preferences
          ({}, {}, {}, {}, {}, {preference, preference});
      }
    , std::runtime_error ("the preferences must be distinct!")
    );
}

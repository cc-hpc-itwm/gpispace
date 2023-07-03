// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <implementation/finance/asianopt.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (verify_stability_of_parameter_set)
{
  asianopt::Parameters parameters
    { 7000.0
    , 85.0
    , 1.02
    , 0.2
    , 0.05
    , 0.0
    , 1
    , 50.0
    , asianopt::FixC
    , false
    };

  unsigned long const number_of_rolls (100000);
  unsigned long seed (3134UL);

  auto const roll_result (asianopt::roll (parameters, number_of_rolls, seed));

  asianopt::Reduced const initial_state {0.0, 0.0};
  auto const reduced (asianopt::reduce (initial_state, roll_result));

  auto const result
    (asianopt::post_process (number_of_rolls, reduced, parameters));

  BOOST_REQUIRE_CLOSE (725582881.627464, reduced.sum_expected_value, 0.0000001);
  BOOST_REQUIRE_CLOSE (5268457936478.763672, reduced.sum_variance, 0.0000001);
  BOOST_REQUIRE_CLOSE (6895.059361, result.price, 0.000001);
  BOOST_REQUIRE_CLOSE (0.582138, result.std_dev, 0.0001);
}

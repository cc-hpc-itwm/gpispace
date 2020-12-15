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

#include <implementation/asian/asianopt.h>

#include <cmath>
#include <cstdio>
#include <limits>

bool roughly_equal (double x, double y)
{
  return std::abs (x - y) <= 0.000001;
}

int main (int, char**)
{
  asian::param_t stParam { 7000.0
                         , 85.0
                         , 1.02
                         , 0.2
                         , 0.05
                         , 0.0
                         , 1
                         , 50.0
                         , asian::FixC
                         , false
                         };

  unsigned long const number_of_rolls (100000);
  unsigned long seed (3134UL);

  asian::roll_result_type const roll_result
    (asian::roll (stParam, number_of_rolls, seed));

  asian::reduced_type const initial_state {0.0, 0.0};
  asian::reduced_type const reduced (asian::reduce (initial_state, roll_result));

  asian::result_type const result
    (asian::post_process (number_of_rolls, reduced, stParam));

  printf ("Sum1 = %lf\n", reduced.sum1);
  printf ("Sum2 = %lf\n", reduced.sum2);
  printf ("price = %lf\n", result.price);
  printf ("std-dev = %lf\n", result.std_dev);

  double const expected_Sum1 (725582881.627464);
  double const expected_Sum2 (5268457936478.763672);
  double const expected_price (6895.059361);
  double const expected_std_dev (0.582138);

  return roughly_equal (expected_Sum1, reduced.sum1)
      && roughly_equal (expected_Sum2, reduced.sum2)
      && roughly_equal (expected_price, result.price)
      && roughly_equal (expected_std_dev, result.std_dev)
    ? 0 : -1;
}

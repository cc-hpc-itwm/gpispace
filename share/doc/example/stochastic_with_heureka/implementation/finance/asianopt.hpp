// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#pragma once

namespace asianopt
{
  // Asian - Option
  //
  // Fixed-Strike-Call    : max[A(T)-K,0]
  // Fixed-Strike-Put     : max[K-A(T),0]
  // Floating-Strike-Call : max[S(T)-A(T),0]
  // Floating-Strike-Put  : max[A(T)-S(T),0]
  // A(T) arithmetic average , A(T) = A(t1)+A(t2)+....+A(tn)
  //
  // MonteCarlo with geometric average as control variate

  enum OptionType
  {
    // Floating-Strike-Call
    FloC,
    // Floating-Strike-Put
    FloP,
    // Fixed-Strike-Call
    FixC,
    // Fixed-Strike-Put
    FixP,
  };

  struct Parameters
  {
    // stock price
    double S;
    // strike
    double K;
    // maturity
    double T;

    // volatility
    double Sigma;
    // interest rate
    double r;
    double d;

    int first_fixing;
    double fixings_per_year;

    OptionType type;
    bool control_variate;

    Parameters() = default;
    Parameters ( double S_
               , double K_
               , double T_
               , double sigma_
               , double r_
               , double d_
               , int first_fixing_
               , double fixings_per_year_
               , OptionType type_
               , bool control_variate_
               );
  };

  struct RollResult
  {
    double sum_expected_value;
    double sum_variance;
  };
  using Reduced = RollResult;
  struct Result
  {
    double price;
    double std_dev;
  };

  RollResult roll
    (Parameters, unsigned long number_of_rolls, unsigned long seed);
  Reduced reduce (Reduced, RollResult);
  Result post_process (unsigned long number_of_rolls, Reduced, Parameters);
}

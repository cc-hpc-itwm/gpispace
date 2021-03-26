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

#include <interface.hpp>

#include <cmath>
#include <random>
#include <tuple>
#include <utility>

extern "C"
  std::pair<we::type::bytearray, bool> stochastic_with_heureka_roll_and_heureka
    (unsigned long n, unsigned long seed, we::type::bytearray)
{
  std::mt19937 generator (seed);
  std::uniform_real_distribution<double> random_number (-1, 1);

  unsigned long in (0);

  for (unsigned long i (0); i < n; ++ i)
  {
    double const x (random_number (generator));
    double const y (random_number (generator));
    double const l (std::sqrt (x * x + y * y));

    if (l <= 1)
    {
      ++in;
    }
  }

  return {we::type::bytearray {std::make_pair (in, n)}, false};
};

extern "C"
  we::type::bytearray stochastic_with_heureka_reduce
    ( we::type::bytearray partial_resultL_bytearray
    , we::type::bytearray partial_resultR_bytearray
    , we::type::bytearray
    )
{
  std::pair<unsigned long, unsigned long> partial_resultL;
  std::pair<unsigned long, unsigned long> partial_resultR;
  partial_resultL_bytearray.copy (&partial_resultL);
  partial_resultR_bytearray.copy (&partial_resultR);

  return we::type::bytearray
    { std::make_pair ( partial_resultL.first + partial_resultR.first
                     , partial_resultL.second + partial_resultR.second
                     )
    };
}

extern "C"
  we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long number_of_rolls
    , we::type::bytearray result_bytearray
    , we::type::bytearray
    )
{
  std::pair<unsigned long, unsigned long> result;
  result_bytearray.copy (&result);

  unsigned long gcd (result.second);
  unsigned long rem (result.first);

  while (rem != 0)
  {
    unsigned long const r (gcd % rem);
    gcd = rem;
    rem = r;
  }

  std::pair<unsigned long, unsigned long> const reduced
    {result.first / gcd, result.second / gcd};

  double const res (4.0 * double (result.first) / double (result.second));

  return we::type::bytearray {std::make_tuple (result, reduced, res)};
}

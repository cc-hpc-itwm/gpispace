// Copyright (C) 2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <interface.hpp>

#include "pi.hpp"
#include <cmath>
#include <numeric>
#include <random>
#include <tuple>
#include <utility>

extern "C"
  std::pair<gspc::we::type::bytearray, bool> stochastic_with_heureka_roll_and_heureka
    (unsigned long n, unsigned long seed, gspc::we::type::bytearray)
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

  using gspc::share::example::stochastic_with_heurake::pi::PartialResult;

  return {gspc::we::type::bytearray {PartialResult {in, n}}, false};
};

extern "C"
  gspc::we::type::bytearray stochastic_with_heureka_reduce
    ( gspc::we::type::bytearray partial_resultL_bytearray
    , gspc::we::type::bytearray partial_resultR_bytearray
    , gspc::we::type::bytearray
    )
{
  using gspc::share::example::stochastic_with_heurake::pi::PartialResult;

  PartialResult partial_resultL;
  PartialResult partial_resultR;
  partial_resultL_bytearray.copy (&partial_resultL);
  partial_resultR_bytearray.copy (&partial_resultR);

  return gspc::we::type::bytearray
    { PartialResult { partial_resultL.in + partial_resultR.in
                    , partial_resultL.n + partial_resultR.n
                    }
    };
}

extern "C"
  gspc::we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long number_of_rolls
    , gspc::we::type::bytearray result_bytearray
    , gspc::we::type::bytearray
    )
{
  using gspc::share::example::stochastic_with_heurake::pi::PartialResult;

  PartialResult result;
  result_bytearray.copy (&result);

  auto const gcd {std::gcd (result.in, result.n)};

  PartialResult const reduced {result.in / gcd, result.n / gcd};

  double const res (4.0 * double (result.in) / double (result.n));

  using gspc::share::example::stochastic_with_heurake::pi::Result;

  return gspc::we::type::bytearray {Result {result, reduced, res}};
}

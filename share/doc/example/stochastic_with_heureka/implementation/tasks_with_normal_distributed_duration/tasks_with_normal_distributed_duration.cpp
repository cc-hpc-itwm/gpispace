// Copyright (C) 2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <interface.hpp>

#include "Parameters.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <thread>
#include <utility>

extern "C"
  std::pair<gspc::we::type::bytearray, bool> stochastic_with_heureka_roll_and_heureka
    (unsigned long n, unsigned long seed, gspc::we::type::bytearray user_data)
{
  std::mt19937 generator (seed);
  using gspc::share::example::stochastic_with_heurake::tasks::Parameters;
  Parameters parameters;
  user_data.copy (&parameters);
  std::normal_distribution<float> random_number
    (parameters.mean, parameters.deviation);

  auto const duration (std::max (0.0f, random_number (generator)));

  std::this_thread::sleep_for
    (std::chrono::milliseconds (std::lround (duration)));

  return {gspc::we::type::bytearray {n}, false};
};

extern "C"
  gspc::we::type::bytearray stochastic_with_heureka_reduce
    ( gspc::we::type::bytearray partial_resultL_bytearray
    , gspc::we::type::bytearray partial_resultR_bytearray
    , gspc::we::type::bytearray
    )
{
  unsigned long partial_resultL;
  unsigned long partial_resultR;
  partial_resultL_bytearray.copy (&partial_resultL);
  partial_resultR_bytearray.copy (&partial_resultR);

  return gspc::we::type::bytearray {partial_resultL + partial_resultR};
}

extern "C"
  gspc::we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long number_of_rolls
    , gspc::we::type::bytearray result_bytearray
    , gspc::we::type::bytearray
    )
{
  return result_bytearray;
}

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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <thread>
#include <utility>

extern "C"
  std::pair<we::type::bytearray, bool> stochastic_with_heureka_roll_and_heureka
    (unsigned long n, unsigned long seed, we::type::bytearray user_data)
{
  std::mt19937 generator (seed);
  std::pair<unsigned int, unsigned int> parameters;
  user_data.copy (&parameters);
  std::normal_distribution<float> random_number
    (parameters.first, parameters.second);

  auto const duration (std::max (0.0f, random_number (generator)));

  std::this_thread::sleep_for
    (std::chrono::milliseconds (std::lround (duration)));

  return {we::type::bytearray {n}, false};
};

extern "C"
  we::type::bytearray stochastic_with_heureka_reduce
    ( we::type::bytearray partial_resultL_bytearray
    , we::type::bytearray partial_resultR_bytearray
    , we::type::bytearray
    )
{
  unsigned long partial_resultL;
  unsigned long partial_resultR;
  partial_resultL_bytearray.copy (&partial_resultL);
  partial_resultR_bytearray.copy (&partial_resultR);

  return we::type::bytearray {partial_resultL + partial_resultR};
}

extern "C"
  we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long number_of_rolls
    , we::type::bytearray result_bytearray
    , we::type::bytearray
    )
{
  return result_bytearray;
}

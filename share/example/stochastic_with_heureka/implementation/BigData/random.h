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

#pragma once

#include <random>
#include <cassert>

struct crand_engine
{
  using result_type = unsigned int;
  constexpr result_type min() const { return 0; }
  constexpr result_type max() const { return RAND_MAX; }
  result_type _seed;
  constexpr crand_engine (unsigned long seed = 0)
    : _seed (seed)
  {}
  result_type operator()()
  {
    return rand_r (&_seed);
  }
};

template<typename T>
struct negative_one_to_one_bad_distribution
{
  negative_one_to_one_bad_distribution (T min, T max)
  {
    assert (min == -1.0);
    assert (max == 1.0);
  }
  using result_type = T;
  template<typename Engine>
  result_type operator() (Engine& engine) const
  {
    assert (engine.min() == 0);
    return 2.0 * engine() / T (engine.max()) - 1.0;
  }
};

// A convenient method for generating normal variables,
// G. Marsaglia and T. A. Bray, SIAM Rev. 6, 260–264, 1964
template < typename T
         , typename underlying_distribution
         >
struct marsaglia_normal_distribution
{
private:
  underlying_distribution _uniform;
  bool _has_cached;
  T _cached;

public:
  marsaglia_normal_distribution()
    : _uniform (-1.0, 1.0)
    , _has_cached (false)
    , _cached (0.0)
  {}

  template<typename Engine>
  T operator() (Engine& engine)
  {
    if (_has_cached)
    {
      _has_cached = false;
      return _cached;
    }

    T u1, u2, r;
    do
    {
      u1 = _uniform (engine);
      u2 = _uniform (engine);
      r = u1 * u1 + u2 * u2;
    }
    while (!(r < 1.0 && r != 0.0));

    T const f (sqrt (-2.0 * log (r) / r));
    _cached = u2 * f;
    _has_cached = true;
    return u1 * f;
  }
};

class NormalNumber
{
private:
  // using engine_type = std::random_device;
  // using engine_type = std::mt19937_64;
  using engine_type = crand_engine;

  // using distribution_type = std::normal_distribution<double>;
  // using distribution_type = marsaglia_normal_distribution<double, negative_one_to_one_bad_distribution<double>>;
  using distribution_type = marsaglia_normal_distribution<double, std::uniform_real_distribution<double>>;

  engine_type _engine;
  distribution_type _distribution;

public:
	double operator()()
  {
    return _distribution (_engine);
  }
};

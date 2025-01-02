// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <implementation/finance/barropt.hpp>

#include <interface.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
  // A C++11 <random> style wrapper around rand_r().
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

      T const f (std::sqrt (-2.0 * std::log (r) / r));
      _cached = u2 * f;
      _has_cached = true;
      return u1 * f;
    }
  };
}

using namespace barrieropt;

std::pair<we::type::bytearray, bool>
  stochastic_with_heureka_roll_and_heureka
    ( unsigned long n
    , unsigned long seed
    , we::type::bytearray user_data_bytearray
    )
{
  barrieropt::SingleBarrierOptionMonteCarlo_user_data user_data;
  user_data_bytearray.copy (&user_data);

  double const S (user_data.S);
  double const K (user_data.K);
  double const H (user_data.H);
  double const r (user_data.r);
  double const sigma (user_data.sigma);
  double const T (user_data.T);
  int const TimeSteps (user_data.TimeSteps);
  barrieropt::OptionType const option_type (user_data.option_type);
  barrieropt::BarrierType const barrier_type (user_data.barrier_type);

  if (n < 10)
  {
    throw std::logic_error ("requires: rolls_at_once >= 10");
  }

  // initialization of pseudo-random generator
  // using engine_type = std::mt19937_64;
  using engine_type = crand_engine;
  // using distribution_type = std::normal_distribution<double>;
  // using distribution_type = marsaglia_normal_distribution<double, std::uniform_real_distribution<double>>;
  using distribution_type = marsaglia_normal_distribution<double, negative_one_to_one_bad_distribution<double>>;

  engine_type random_engine (seed);
  distribution_type random_distribution;

  int const FirstFixing = 1;
  int const LastFixing = TimeSteps;

  double const dt (T / TimeSteps);

  // Drift
  double const mu (r-sigma*sigma/2.0);

  double sum_expected_value (0.0);
  double sum_variance (0.0);

  for (unsigned long int j (0); j < n; ++j)
  {
    // stock prices
    double S_t (S);
    double S_T;

    bool barrierConditionIn = false;
    bool barrierConditionOut = true;

    for (long int i (FirstFixing); i < LastFixing; ++i)
    {
      double const StdNorm (random_distribution (random_engine));

      S_T = S_t * std::exp (mu * dt + sigma * std::sqrt (dt) * StdNorm);

      if ((barrier_type == UaO && S_T >= H ) || (barrier_type == DaO && S_T <= H))
      {
        barrierConditionOut = false;
        break;
      }
      if ((barrier_type == UaI && S_T >= H) || (barrier_type == DaI && S_T <= H))
      {
        barrierConditionIn = true;
      }

      S_t = S_T;
    }

    auto const is_and_in_barrier (barrier_type == DaI || barrier_type == UaI);
    auto const is_and_out_barrier (barrier_type == DaO || barrier_type == UaO);
    auto const is_barrier_condition_satisfied
      ( (barrierConditionIn && is_and_in_barrier)
     || (barrierConditionOut && is_and_out_barrier)
      );

    // price
    double value;
    if (!is_barrier_condition_satisfied)
    {
      value = 0.0;
    }
    else if (option_type == Call)
    {
      value = std::max (S_T - K, 0.0);
    }
    else if (option_type == Put)
    {
      value = std::max (K - S_T, 0.0);
    }

    sum_expected_value += value;
    sum_variance += value * value;
  }

  return {we::type::bytearray {std::make_pair (sum_expected_value, sum_variance)}, false};
}

we::type::bytearray stochastic_with_heureka_reduce
  ( we::type::bytearray state_bytearray
  , we::type::bytearray partial_result_bytearray
  , we::type::bytearray
  )
{
  std::pair<double, double> state;
  std::pair<double, double> partial_result;
  state_bytearray.copy (&state);
  partial_result_bytearray.copy (&partial_result);

  return we::type::bytearray
    { std::make_pair ( state.first + partial_result.first
                     , state.second + partial_result.second
                     )
    };
}

we::type::bytearray stochastic_with_heureka_post_process
  ( unsigned long number_of_rolls
  , we::type::bytearray reduced_bytearray
  , we::type::bytearray user_data_bytearray
  )
{
  std::pair<double, double> reduced;
  reduced_bytearray.copy (&reduced);
  SingleBarrierOptionMonteCarlo_user_data user_data;
  user_data_bytearray.copy (&user_data);

  double const temp (std::exp (-user_data.r * user_data.T));
  return we::type::bytearray
    { std::make_pair
      ( temp * (reduced.first / number_of_rolls)
      , temp * std::sqrt ( 1.0 / (number_of_rolls - 1)
                         * ( 1.0 / number_of_rolls * reduced.second
                           - reduced.first * reduced.first
                           / (number_of_rolls * number_of_rolls)
                           )
                         )
      )
    };
}

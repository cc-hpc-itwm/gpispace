// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <implementation/finance/asianopt.hpp>

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace asianopt
{
  RollResult roll ( Parameters parameters
                  , unsigned long number_of_rolls
                  , unsigned long seed
                  )
  {
    using engine_type = std::mt19937_64;
    using distribution_type = std::normal_distribution<double>;

    engine_type random_engine (seed);
    distribution_type random_distribution;

    long const number_of_fixings (parameters.fixings_per_year * parameters.T);
    int const last_fixing (number_of_fixings);

    std::vector<double> times (last_fixing + 1);
    std::vector<double> weights (last_fixing + 1, 1.0);
    weights[0] = 0.0; // never read

    double const dx (parameters.T / 365.0);

    times[0] = 0.0;
    times[number_of_fixings] = parameters.T;
    for (long fixing (number_of_fixings - 1); fixing >= 1; --fixing)
    {
      times[fixing] = times[fixing + 1] - dx;
    }

    // reduced spot
    double const S0 (parameters.S);
    // drift
    double const mu ( parameters.r - parameters.d
                    - parameters.Sigma * parameters.Sigma / 2.0
                    );
    double sum_expected_value (0.0);
    double sum_variance (0.0);
    // multipliers for option types
    double Ak (0.0);
    double Sk (0.0);
    double Kk (0.0);

    double ef_weight (0.0);
    for (int i (parameters.first_fixing); i <= last_fixing; ++i)
    {
      ef_weight += weights[i];
    }

    // coefficients based on type
    switch (parameters.type)
    {
    case FixC:
      Ak =  1.0;
      Sk =  0.0;
      Kk = -1.0;
      break;

    case FixP:
      Ak = -1.0;
      Sk =  0.0;
      Kk =  1.0;
      break;

    case FloP:
      Ak =  1.0;
      Sk = -1.0;
      Kk =  1.0;
      parameters.K = 0.0;
      break;

    case FloC:
      Ak = -1.0;
      Sk =  1.0;
      Kk = -1.0;
      parameters.K = 0.0;
      break;

    default:
      throw std::logic_error ("invalid type");
    };

    for (unsigned long roll (0); roll < number_of_rolls; ++roll)
    {
      double const DeltaT (times[parameters.first_fixing] - times[0]);
      double const weight_T (weights[parameters.first_fixing]);
      double const StdNorm (random_distribution (random_engine));

      // Spot1, Spot2 (AntiThetic)
      auto S1 = std::exp (mu * DeltaT - parameters.Sigma * std::sqrt (DeltaT) * StdNorm);
      auto S2 = std::exp (mu * DeltaT + parameters.Sigma * std::sqrt (DeltaT) * StdNorm);

      // initialization of arithmetic average
      auto A1 = S1 * weight_T;
      auto A2 = S2 * weight_T;

      // initialization of geometric average
      auto GA1 = std::pow (S1, weights[parameters.first_fixing]);
      auto GA2 = std::pow (S2, weights[parameters.first_fixing]);

      for (int i (parameters.first_fixing + 1); i <= last_fixing; ++i)
      {
        double const DeltaT (times[i] - times[i-1]);
        double const G (weights[i]);
        double const StdNorm (random_distribution (random_engine));

        S1 = S1 * std::exp(mu * DeltaT - parameters.Sigma * std::sqrt (DeltaT) * StdNorm);
        S2 = S2 * std::exp(mu * DeltaT + parameters.Sigma * std::sqrt (DeltaT) * StdNorm);

        // arithmetic average
        A1 += S1 * G;
        A2 += S2 * G;

        // geometric average
        GA1 *= std::pow (S1, G);
        GA2 *= std::pow (S2, G);
      }

      // correct weights
      A1 = S0 * A1 / ef_weight;
      A2 = S0 * A2 / ef_weight;

      GA1 = S0 * std::pow (GA1, 1 / ef_weight);
      GA2 = S0 * std::pow (GA2, 1 / ef_weight);

      double value
        ( 0.5 * ( std::max (Sk * S1 * S0 + Ak * A1 + Kk * parameters.K, 0.0)
                + std::max (Sk * S2 * S0 + Ak * A2 + Kk * parameters.K, 0.0)
                )
        );
      if (parameters.control_variate)
      {
        double const valuecv
          ( 0.5 * ( std::max (Sk * S1 * S0 + Ak * GA1 + Kk * parameters.K, 0.0)
                  + std::max (Sk * S2 * S0 + Ak * GA2 + Kk * parameters.K, 0.0)
                  )
          );
        value -= valuecv;
      }

      sum_expected_value += value;
      sum_variance += value * value;
    }

    return {sum_expected_value, sum_variance};
  }

  Reduced reduce (Reduced state, RollResult partial_result)
  {
    return { state.sum_expected_value + partial_result.sum_expected_value
           , state.sum_variance + partial_result.sum_variance
           };
  }
  Result post_process ( unsigned long number_of_rolls
                      , Reduced reduced_result
                      , Parameters parameters
                      )
  {
    double const temp (std::exp (-parameters.r * parameters.T));
    return { temp * (reduced_result.sum_expected_value / number_of_rolls)
           , temp * std::sqrt ( ( reduced_result.sum_variance
                                - reduced_result.sum_expected_value * reduced_result.sum_expected_value
                                / number_of_rolls
                                )
                              / (number_of_rolls * number_of_rolls)
                              )
           };
   }

  namespace
  {
    template<typename T> void require_GT (T value, T than, std::string what)
    {
      if (!(value > than))
      {
        throw std::invalid_argument
          ( "requires: " + what + ": "
          + std::to_string (value) + " > " + std::to_string (than)
          );
      }
    }
    template<typename T> void require_GE (T value, T than, std::string what)
    {
      if (!(value >= than))
      {
        throw std::invalid_argument
          ( "requires: " + what + ": "
          + std::to_string (value) + " >= " + std::to_string (than)
          );
      }
    }
    template<typename T> void require_LT (T value, T than, std::string what)
    {
      if (!(value < than))
      {
        throw std::invalid_argument
          ( "requires: " + what + ": "
          + std::to_string (value) +" < " + std::to_string (than)
          );
      }
    }
  }

  Parameters::Parameters ( double S_
                         , double K_
                         , double T_
                         , double sigma_
                         , double r_
                         , double d_
                         , int first_fixing_
                         , double fixings_per_year_
                         , OptionType type_
                         , bool control_variate_
                         )
    : S (S_)
    , K (K_)
    , T (T_)
    , Sigma (sigma_)
    , r (r_)
    , d (d_)
    , first_fixing (first_fixing_)
    , fixings_per_year (fixings_per_year_)
    , type (type_)
    , control_variate (control_variate_)
  {
    int last_fixing (fixings_per_year * T);

    require_GT (S, 0.0, "S");
    require_GE (K, 0.0, "K");
    require_GT (Sigma, 0.0, "sigma");
    require_GE (r, 0.0, "r");
    require_GT (T, 0.0, "T");
    require_GT (last_fixing, first_fixing, "fixings/yr * T");
    require_GT (first_fixing, 0, "first fixing");
  }
}

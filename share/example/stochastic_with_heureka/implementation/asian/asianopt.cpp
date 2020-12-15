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
#include <random>
#include <stdexcept>
#include <vector>

namespace asian
{
  // ****************************************************************************
  // *****     Asian - Option                                               *****
  // *****                                                                  *****
  // *****     Fixed-Strike-Call   : max[A(T)-K,0]                          *****
  // *****     Fixed-Strike-Put    : max[K-A(T),0]                          *****
  // *****     Floating-Strike-Call: max[S(T)-A(T),0]                       *****
  // *****     Floating-Strike-Put : max[A(T)-S(T),0]                       *****
  // *****     A(T) arithmetisches Mittel , A(T) = A(t1)+A(t2)+....+A(tn)   *****
  // *****                                                                  *****
  // *****     MonteCarlo mit geometrischem Mittel als Controle Variate     *****
  // *****     Quelle: Eigenentwicklung                                     *****
  // ****************************************************************************

  roll_result_type roll ( param_t parameters
                        , unsigned long number_of_rolls
                        , unsigned long seed
                        )
  {
    using engine_type = std::mt19937_64;
    using distribution_type = std::normal_distribution<double>;

    engine_type random_engine (seed);
    distribution_type random_distribution;


    long const AnzahlderFixings (parameters.m_dFixingsProJahr * parameters.m_dT);
    int const LastFixing (AnzahlderFixings);

    std::vector<double> TimeV (LastFixing + 1);
    std::vector<double> GewV (LastFixing + 1, 1.0);
    GewV[0] = 0.0; // never read

  	double const dx (parameters.m_dT / 365.0);

  	TimeV[0] = 0.0;
  	TimeV[AnzahlderFixings] = parameters.m_dT;
    for (long lFixing=(AnzahlderFixings-1); lFixing>=1; lFixing--)
    {
  		TimeV[lFixing] = TimeV[lFixing + 1] - dx;
    }


    // Berechnung des reduzierten Spots;
    double const S0 (parameters.m_dS);

    double S1, S2;    // Spot
    double GA1, GA2;  // Geometrische Mittel
    double A1, A2;    // Arithmetisce Mittel

    double const mu ( parameters.m_dr - parameters.m_dd
                    - parameters.m_dSigma * parameters.m_dSigma / 2.0
                    ); // Drift
    double Sum1 (0.0);          // Summen zur Berechnung der Varianz und des Erwartungswerts
    double Sum2 (0.0);
    double Ak (0.0);        // Multiplikatoren f�r Optionstypen
    double Sk (0.0);        // Multiplikatoren f�r Optionstypen
    double Kk (0.0);        // Multiplikatoren f�r Optionstypen

    double EfGew (0.0);
    for (int i (parameters.m_nFirstFixing); i <= LastFixing; ++i)
    {
      EfGew += GewV[i];
    }

    // Festlegung der Berechnungskoeffizienten
    switch (parameters.type)
    {
    case FixC:
      Ak = 1.0;
      Sk = 0.0;
      Kk = -1.0;
      break;

    case FixP:
      Ak = -1.0;
      Sk = 0.0;
      Kk = 1.0;
      break;

    case FloP:
      Ak = 1.0;
      Sk =-1.0;
      Kk = 1.0;
      parameters.m_dK = 0.0;
      break;

    case FloC:
      Ak = -1.0;
      Sk = 1.0;
      Kk = -1.0;
      parameters.m_dK = 0.0;
      break;

    default:
      throw std::logic_error ("invalid type");
    };

    for (unsigned long roll (0); roll < number_of_rolls; ++roll)
    {
      double const DeltaT (TimeV[parameters.m_nFirstFixing] - TimeV[0]);
      double const GewT (GewV[parameters.m_nFirstFixing]);
      double const StdNorm (random_distribution (random_engine));

      // Spot1, Spot2 (AntiThetic)
      S1 = exp (mu * DeltaT - parameters.m_dSigma * sqrt (DeltaT) * StdNorm);
      S2 = exp (mu * DeltaT + parameters.m_dSigma * sqrt (DeltaT) * StdNorm);

      // Initialisierung des Aritmetischen Mittels
      A1 = S1 * GewT;
      A2 = S2 * GewT;

      // Initialisierung des Geomtrischen Mittels
      GA1 = pow (S1, GewV[parameters.m_nFirstFixing]);
      GA2 = pow (S2, GewV[parameters.m_nFirstFixing]);

      for (int i (parameters.m_nFirstFixing + 1); i <= LastFixing; ++i)
      {
        double const DeltaT (TimeV[i] - TimeV[i-1]);
        double const G (GewV[i]);
        double const StdNorm (random_distribution (random_engine));

        S1 = S1 * exp(mu * DeltaT - parameters.m_dSigma * sqrt (DeltaT) * StdNorm);
        S2 = S2 * exp(mu * DeltaT + parameters.m_dSigma * sqrt (DeltaT) * StdNorm);

        // Arithmetisches Mittel
        A1 += S1 * G;
        A2 += S2 * G;

        // Geometrisches Mittel
        GA1 *= pow (S1, G);
        GA2 *= pow (S2, G);
      }

      // Korrekte Gewichtung
      A1 = S0 * A1 / EfGew;
      A2 = S0 * A2 / EfGew;

      GA1 = S0 * pow (GA1, 1 / EfGew);
      GA2 = S0 * pow (GA2, 1 / EfGew);

      double value
        ( 0.5 * ( std::max (Sk * S1 * S0 + Ak * A1 + Kk * parameters.m_dK, 0.0)
                + std::max (Sk * S2 * S0 + Ak * A2 + Kk * parameters.m_dK, 0.0)
                )
        );
      if (parameters.controle_variate)
      {
        double const valuecv
          ( 0.5 * ( std::max (Sk * S1 * S0 + Ak * GA1 + Kk * parameters.m_dK, 0.0)
                  + std::max (Sk * S2 * S0 + Ak * GA2 + Kk * parameters.m_dK, 0.0)
                  )
          );
        value -= valuecv;
      }

      Sum1 += value;
      Sum2 += value * value;
    }

  	return {Sum1, Sum2};
  }

  reduced_type reduce (reduced_type state, roll_result_type partial_result)
  {
    return {state.sum1 + partial_result.sum1, state.sum2 + partial_result.sum2};
  }
  result_type post_process ( unsigned long number_of_rolls
                           , reduced_type reduced_result
                           , param_t parameters
                           )
  {
    double const temp (std::exp (-parameters.m_dr * parameters.m_dT));
    return { temp * (reduced_result.sum1 / number_of_rolls)
           , temp * std::sqrt ( ( reduced_result.sum2
                                - reduced_result.sum1 * reduced_result.sum1
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

  param_t::param_t ( double S
                   , double K
                   , double T
                   , double sigma
                   , double r
                   , double d
                   , int first_fixing
                   , double fixings_per_year
                   , AsianTyp type_
                   , bool controle_variate_
                   )
    : m_dS (S)
    , m_dK (K)
    , m_dT (T)
    , m_dSigma (sigma)
    , m_dr (r)
    , m_dd (d)
    , m_nFirstFixing (first_fixing)
    , m_dFixingsProJahr (fixings_per_year)
    , type (type_)
    , controle_variate (controle_variate_)
  {
  	int LastFixing (m_dFixingsProJahr * m_dT);

    require_GT (m_dS, 0.0, "S");
    require_GE (m_dK, 0.0, "K");
    require_GT (m_dSigma, 0.0, "sigma");
    require_GE (m_dr, 0.0, "r");
    require_GT (m_dT, 0.0, "T");
    require_GT (LastFixing, m_nFirstFixing, "fixings/yr * T");
    require_GT (m_nFirstFixing, 0, "first fixing");
  }
}

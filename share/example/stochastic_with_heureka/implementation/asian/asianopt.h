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

namespace asian
{
  enum AsianTyp
  {
    FloC,
    FloP,
    FixC,
    FixP
  };

  struct param_t
  {
    double m_dS; // Aktienpreis
    double m_dK; // Strike
    double m_dT; // Faelligkeit

    double m_dSigma; // Volatilitaet
    double m_dr; // Zinsrate
    double m_dd;
    int m_nFirstFixing;
    double m_dFixingsProJahr;

    AsianTyp type;
    bool controle_variate;

    param_t() = default;
    param_t ( double S
            , double K
            , double T
            , double sigma
            , double r
            , double d
            , int first_fixing
            , double fixings_per_year
            , AsianTyp type
            , bool controle_variate
            );
  };

  struct roll_result_type
  {
    double sum1;
    double sum2;
  };
  using reduced_type = roll_result_type;
  struct result_type
  {
    double price;
    double std_dev;
  };

  roll_result_type roll
    (param_t, unsigned long number_of_rolls, unsigned long seed);
  reduced_type reduce (reduced_type, roll_result_type);
  result_type post_process
    (unsigned long number_of_rolls, reduced_type, param_t);
}

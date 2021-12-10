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

namespace barrieropt
{
  // Vanilla-Barrier
  //
  // Call: max(S-K,0)*barrier_condiiton
  // Put:  max(K-S,0)*barrier_condiiton
  //
  // Method: MonteCarlo

  enum OptionType
  {
    Call,
    Put,
  };

  enum BarrierType
  {
    // Down-and-Out
    DaO,
    // Down-and-In
    DaI,
    // Up-and-Out
    UaO,
    // Up-and-In
    UaI,
  };

  struct SingleBarrierOptionMonteCarlo_user_data
  {
    double S; // underlying
    double K; // strike price
    double H; // barrier
    double r; // interest rate
    double sigma; // volatility
    double T; // timeframe
    int TimeSteps; // discretization
    OptionType option_type;
    BarrierType barrier_type;
  };

  struct SingleBarrierOptionMonteCarlo_result_type
  {
    double price;
    double std_dev;
  };
}

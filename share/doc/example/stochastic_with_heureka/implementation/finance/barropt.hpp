// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

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

#include "basics.h"

enum BarrierTyp {DaO,DaI,UaO,UaI};

#include "we/type/bytearray.hpp"

double SingleBarrierOption( double S,
						    double K,
						    double H,
						    double r,
						    double sigma,
						    double T,
							double timeSteps,
   						    OptionTyp OptA,
						    BarrierTyp BarA);


struct SingleBarrierOptionMonteCarlo_user_data
{
  double S; // Basiswert
  double K; // Ausübungspreis
  double H; // Barriere
  double r; // Zinsrate
  double sigma; // Volatilität
  double T; // Laufzeit
  int TimeSteps; // Diskretisierung
  OptionTyp OptA; //Optionsart: Kaufoption (Call), Verkaufsoption (Put)
  BarrierTyp BarA; // Barrieretyp: Down-and-Out (DaO), Down-and-In (DoI), Up-and-Out (UaO), Up-and-In (UaI)
};

struct SingleBarrierOptionMonteCarlo_result_type
{
  double price;
  double std_dev;
};

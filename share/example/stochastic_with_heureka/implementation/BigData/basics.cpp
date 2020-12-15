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

#include "prob.h"
#include "random.h"
#include "basics.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <algorithm>

// ****************************************************************************
// *****     European - Vanilla Option                                    *****
// *****                                                                  *****
// *****     Call: max[S(T)-K,0]                                          *****
// *****     Put : max[K-S(T),0]                                          *****
// *****                                                                  *****
// *****     Quelle: Hull / Espen Gaarder Haug                            *****
// ****************************************************************************


double BlackScholesVanilla ( double S,
							 double K,
						     double r,
							 double sigma,
							 double T,
							 OptionTyp Art)

{
	double d1,d2,value;

	if ( S <= 0 || r<0 || sigma<=0.0 || T<0 ) return -999.999;
	if (T>0.0)
		{
				d1 = (log(S/K)+(r+sigma*sigma/2.0)*T)/(sigma*sqrt(T));
				d2 = d1-sigma*sqrt(T);
				if (Art==Call)	{value = S*N(d1)-K*exp(-r*T)*N(d2);}
				else            {value = K*exp(-r*T)*N(-d2)-S*N(-d1);};

		}
	 else
		{
      if (Art==Call) {value = std::max(S-K,0.0);}
      else           {value = std::max(K-S,0.0);};
		};
	return value;

};

// ****************************************************************************
// *****     European - Vanilla Option                                    *****
// *****                                                                  *****
// *****     Call: max[S(T)-K,0]                                          *****
// *****     Put : max[K-S(T),0]                                          *****
// *****                                                                  *****
// *****     Methode: Monte Carlo			                              *****
// ****************************************************************************


double BlackScholesVanillaMonteCarlo ( double &result,
			 						   double &stdDev,
			 						   double S,
			 						   double K,
									   double T,
									   double sigma,
									   double r,
									   long int n,
									   OptionTyp Art)

{

    if (S<=0 || K<=0 || r<0) return -999.999;
	if (sigma <= 0 || T < 0  || n < 10 ) return -888.888;


	// Initialisierung des Pseudo-Zufallsgenerators

	NormalNumber Norm;


	double StdNorm;        // Variable für Zufallszahl

	double mu = (r-sigma*sigma/2.0); // Drift
	double Sum1=0;          // Summe zur Berechnung des Erwartungswerts
	double Sum2=0;			// Summe zur Berechnung der Varianz
	double variance;         // Varianz
	double value = 0.0;		// Preis
	double S_T;				// Aktienpreis am Laufzeitende

	if (T>0.0)
	{
	for (long int j=1; j<=n; j++)
		  {

			 StdNorm = Norm();

			 S_T = S*exp(mu*T+sigma*sqrt(T)*StdNorm);

			 if (Art == Call)
				 value = std::max(S_T - K,0.0);
			 else value = std::max(K-S_T, 0.0);



			 Sum1 +=value;
			 Sum2 +=value*value;


		};

	value    = exp(-r*T) * ( Sum1/n );
	variance = ( 1.0/(n-1)*(1.0/n*Sum2-Sum1*Sum1/n/n) );

	result=value;
	stdDev=sqrt(variance)*exp(-r*T);
	} else
		{
			stdDev = 0.0;
			if (Art==Call) {result = std::max(S-K,0.0);}
			else           {result = std::max(K-S,0.0);};
		};
	return (result);
};

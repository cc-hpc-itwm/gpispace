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

#include "barropt.h"
#include "prob.h"
#include "random.h"
#include "basics.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// PAYOFF-function Pointer

double (*PAYOFF)  (double S, double K, double H);
double (*PAYOFF2) (double S, double K);

// PAYOFF-functions

double PutDaO (double S, double K, double H)
{
  if(S>H) {return (std::max(K-S,0.0));} else {return 0.0;};
};

double CallDaO (double S, double K, double H)
{
	if(S>H) {return (std::max(S-K,0.0));} else {return 0.0;};
};

double PutUaO (double S, double K, double H)
{
	if(S<H) {return (std::max(K-S,0.0));} else {return 0.0;};
};

double CallUaO (double S, double K, double H)
{
	if(S<H) {return (std::max(S-K,0.0));} else {return 0.0;};
};



//*******************************************************************************
//******         Vanilla-Barrier							                *****
//******                                                                    *****
//******         Call: max(S-K,0)*Barrierebedingung                         *****
//******         Put:  max(K-S,0)*Barrierebedingung                         *****
//******                                                                    *****
//******         Methode: Analytische Lösung aus Zhang,Wilmott,Korn         *****
//******                                                                    *****
//*******************************************************************************





double SingleBarrierOption ( double S,
					         double K,
						     double H,
						     double r,
						     double sigma,
						     double T,
							 double timeSteps,
						     OptionTyp OptA,
						     BarrierTyp BarA)


{
  double value;

  if ( S<=0 || K<=0 || H<=0 || r<0 || sigma<=0 ||  T<0 ) {return -999.999;};



  if (T==0.0) // Beachte T == 0 !!!
  {
	 switch( OptA)
	  {
		case Call:
			if( BarA==DaO || BarA==DaI ) {PAYOFF = CallDaO; break;};
			if( BarA==UaO || BarA==UaI ) {PAYOFF = CallUaO; break;};
		case Put:
			if( BarA==DaO || BarA==DaI ) {PAYOFF = PutDaO;  break;};
			if( BarA==UaO || BarA==UaI ) {PAYOFF = PutUaO;  break;};
		};
	  value = PAYOFF(S,K,H);
	  if ( BarA==DaI || BarA==UaI )
			{return (BlackScholesVanilla(S,K,r,sigma,T,OptA)-value);}
	  else
			{return (value);};

  };



	double Black = BlackScholesVanilla(S,K,r,sigma,T,OptA);


	if ((S<=H) && (BarA==DaO)) {return 0;};
	if ((S<=H) && (BarA==DaI)) {return Black;};
	if ((S>=H) && (BarA==UaO)) {return 0;};
	if ((S>=H) && (BarA==UaI)) {return Black;};


	double StdCall= BlackScholesVanilla(S,K,r,sigma,T,Call);
	double StdPut = BlackScholesVanilla(S,K,r,sigma,T,Put);

	double HK,dob,dun,help;
	double C1,C2,P1,P2;
    double d1,d2;

	double la  = (r+sigma*sigma/2.0)/(sigma*sigma);
	double swt = sigma*sqrt(T);
    double p1  = pow(H/S,2*r/(sigma*sigma)+1.0);
	double p2  = pow(H/S,2*r/(sigma*sigma)-1.0);
	double dt = T/timeSteps;
	double H_original = H;


	// *************************
	// *** Calculating Call  ***
	// *************************

	if (OptA==Call)
	{
		if ((BarA==DaI) || (BarA==DaO))
		{
			// Korn

			// Barriereverschiebung aufgrund Diskretisierung
			H = H * exp(-0.5826 *sigma * sqrt(dt));

			if (K<H_original)
			{
				d1  = ( log(S/H) + (r+sigma*sigma/2.0) * T ) / ( sigma * sqrt(T) );
				d2  = ( log(H/S) + (r+sigma*sigma/2.0) * T ) / ( sigma * sqrt(T) );
			    value   =  S  * N(d1) - K * exp(-r*T) * N(d1-swt);
				value  += -S  * pow(H/S,2.0*r/(sigma*sigma)+1.0) * N(d2);
				value  +=  K * exp(-r*T) * pow(H/S,2.0*r/(sigma*sigma)-1.0) * N(d2-swt);

			}

			// Wilmott

			else
			{
				d1  = ( log(S/K) + (r+sigma*sigma/2.0) * T ) / ( sigma * sqrt(T) );
				d2  = ( log(S*K/(H*H)) - (r-sigma*sigma/2.0) * T ) / ( sigma * sqrt(T) );

			    value   =  S  * N(d1) - K * exp(-r*T) * N(d1-swt);
				value  += -S  * pow(H/S,2.0*r/(sigma*sigma)+1.0) * (1.0-N(d2-swt));
				value  +=  K * exp(-r*T) * pow(H/S,2.0*r/(sigma*sigma)-1.0) * (1.0-N(d2));

            };


			if (BarA==DaI) {value = StdCall-value;};
		}
		else
		{
			// Zhang: Exotic options

			// Barriereverschiebung aufgrund Diskretisierung
			H = H * exp(0.5826 *sigma * sqrt(dt));

			HK=std::max(H,K);
			dob = (log(H/S)+(r-sigma*sigma/2.0)*T)/(sigma*sqrt(T));
			dun = (log(S/HK)+(r-sigma*sigma/2.0)*T)/(sigma*sqrt(T));
			value = 0.0;

			if (H_original>K)
				{
					P1=BlackScholesVanilla(H*H/S,K,r,sigma,T,Put);
					P2=BlackScholesVanilla(H*H/S,H,r,sigma,T,Put);
					help=(H-K)*exp(-r*T)*N(-dob);
					value = pow(H/S,2.0*(la-1.0))*(P1-P2+help);
				};

			value += BlackScholesVanilla(S,HK,r,sigma,T,Call)+(HK-K)*exp(-r*T)*N(dun);


			 if (BarA==UaO) {value = StdCall-value;};

		};
	};


	// ************************
	// *** Calculating Puts ***
	// ************************

	if (OptA==Put)
		{
			if ((BarA==DaO) || (BarA==DaI))
			{
				// Zhang: Exotic options

				// Barriereverschiebung aufgrund Diskretisierung
				H = H * exp(-0.5826 *sigma * sqrt(dt));

				HK  =  std::min(H,K);
				dob = (log(H/S)+(r-sigma*sigma/2.0)*T)/(sigma*sqrt(T));
				dun = (log(S/HK)+(r-sigma*sigma/2.0)*T)/(sigma*sqrt(T));
				value = 0.0;

				if (K>H_original)
					{
						C1=BlackScholesVanilla(H*H/S,K,r,sigma,T,Call);
						C2=BlackScholesVanilla(H*H/S,H,r,sigma,T,Call);
						help=(H-K)*exp(-r*T)*N(dob);
						value = pow(H/S,2.0*(la-1.0))*(C1-C2-help);
					};

				 value += BlackScholesVanilla(S,HK,r,sigma,T,Put)-(HK-K)*exp(-r*T)*N(-dun);


				 if (BarA==DaO) {value = StdPut-value;};

			}
			else
			{
				// Zhang: Exotic options

				// Barriereverschiebung aufgrund Diskretisierung
				H = H * exp(0.5826 *sigma * sqrt(dt));
				HK  = std::min(H,K);

				P1  = BlackScholesVanilla(H*H/S,HK,r,sigma,T,Put);
				dob = ( log(H*H/(S*HK)) + (r-sigma*sigma/2.0)*T) / ( sigma * sqrt(T) );
				dun = ( log(S/H) + (r-sigma*sigma/2.0)*T) / ( sigma * sqrt(T) );

			    value = pow(H/S,2.0*(la-1.0)) * ( P1 - (HK-K) * exp(-r*T) * N(-dob) );
                if (K>H_original)
				{
					value += BlackScholesVanilla(S,K,r,sigma,T,Call) - BlackScholesVanilla(S,H,r,sigma,T,Call);
					value += -(H-K)*exp(-r*T)*N(dun);
				};


                if (BarA==UaO) {value = StdPut-value;};


			 };
		};



	return (value);


};

//*******************************************************************************
//******         Vanilla-Barrier							                *****
//******                                                                    *****
//******         Call: max(S-K,0)*Barrierebedingung                         *****
//******         Put:  max(K-S,0)*Barrierebedingung                         *****
//******                                                                    *****
//******         Methode: MonteCarlo							            *****
//******                                                                    *****
//*******************************************************************************

extern "C"
std::pair<we::type::bytearray, bool>
  stochastic_with_heureka_roll_and_heureka
    ( unsigned long n
    , unsigned long seed
    , we::type::bytearray user_data_bytearray
    )
{
  SingleBarrierOptionMonteCarlo_user_data user_data;
  user_data_bytearray.copy (&user_data);

  double const S (user_data.S);
  double const K (user_data.K);
  double const H (user_data.H);
  double const r (user_data.r);
  double const sigma (user_data.sigma);
  double const T (user_data.T);
  int const TimeSteps (user_data.TimeSteps);
  OptionTyp const OptA (user_data.OptA);
  BarrierTyp const BarA (user_data.BarA);

  if ( n < 10 )
  {
    throw std::logic_error ("requires: rolls_at_once >= 10");
  }



	// Initialisierung des Pseudo-Zufallsgenerators
  // using engine_type = std::mt19937_64;
  using engine_type = crand_engine;
  // using distribution_type = std::normal_distribution<double>;
  // using distribution_type = marsaglia_normal_distribution<double, std::uniform_real_distribution<double>>;
  using distribution_type = marsaglia_normal_distribution<double, negative_one_to_one_bad_distribution<double>>;

  engine_type random_engine (seed);
  distribution_type random_distribution;

	int FirstFixing = 1;
	int LastFixing = TimeSteps;

	std::vector<double> TimeV(LastFixing+1);

	double dt = T/TimeSteps;


	double mu = (r-sigma*sigma/2.0);	// Drift
	double Sum1=0;						// Summe zur Berechnung des Erwartungswerts
	double Sum2=0;						// Summe zur Berechnung der Varianz
	double variance;					// Varianz
	double value = 0.0;					// Preis
	double S_T, S_t;					// Aktienpreise

	bool barrierConditionIn;
	bool barrierConditionOut;
	for (unsigned long int j=1; j<=n; j++)
		  {

			 S_t= S;
			 barrierConditionIn = false;
			 barrierConditionOut = true;

			 for ( long int i=(FirstFixing+1); i<=LastFixing; i++)
				 {

           double StdNorm;						// Variable für Zufallszahl
					StdNorm = random_distribution (random_engine);

					S_T = S_t*exp(mu*dt+sigma*sqrt(dt)*StdNorm);

					if ((BarA == UaO && S_T >= H ) || (BarA == DaO && S_T <= H)){
					barrierConditionOut = false;
					break;
					}
					if ((BarA == UaI && S_T >= H) || (BarA == DaI && S_T <= H)){
						barrierConditionIn = true;
					}

					S_t = S_T;
				 };

			 if (barrierConditionOut == false){
				 value =0.0;
			 }

		      if (OptA == Call && ((barrierConditionIn == true && (BarA == DaI || BarA == UaI )) || ( barrierConditionOut == true && (BarA == DaO || BarA == UaO)))){
            value = std::max(S_T -K, 0.0);
			  }
			  else if (OptA == Put && ((barrierConditionIn == true && (BarA == DaI || BarA == UaI )) || ( barrierConditionOut == true && (BarA == DaO || BarA == UaO)))){
          value = std::max(K-S_T, 0.0);
			  }
			  else{
			  value = 0.0;
			  }

			 Sum1 +=value;
			 Sum2 +=value*value;


		};

  return {we::type::bytearray {std::make_pair (Sum1, Sum2)}, false};
};

extern "C"
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

extern "C"
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

  double const temp (exp (-user_data.r * user_data.T));
  return we::type::bytearray
    { std::make_pair
      ( temp * (reduced.first / number_of_rolls)
      , temp * sqrt ( 1.0 / (number_of_rolls - 1)
                    * ( 1.0 / number_of_rolls * reduced.second
                      - reduced.first * reduced.first
                      / (number_of_rolls * number_of_rolls)
                      )
                    )
      )
    };
}

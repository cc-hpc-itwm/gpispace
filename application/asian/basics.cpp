//#include "StdAfx.h"
#include "prob.h"
#include "random.h"
#include "basics.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

//*******************************************************************************************
//******                    Definition der min/max - Funktion                          ******
//*******************************************************************************************



// #ifdef __cplusplus

#ifndef __maxmin
#define __maxmin

	double max (double value1, double value2);

	double max(double value1, double value2)
	{
		return ( (value1 > value2) ? value1 : value2);
	}

	double min (double value1, double value2);

	double min(double value1, double value2)
	{
		return ( (value1 < value2) ? value1 : value2);
	}

#endif



	


int imax(int value1, int value2)
{
	return ( (value1 > value2) ? value1 : value2);
}

int imin (int value1, int value2);

int imin(int value1, int value2)
{
	return ( (value1 < value2) ? value1 : value2);
}


// ******************************************************************************************
// ******                           Dividenden-Klasee                                  ******
// ******************************************************************************************


// Constructors


Dividenden::Dividenden()
{
	Dw = new double[1];
	t =  new double[1];
	Dw[0] = 0.0;
	t[0] = 0.0;
	Nr = 1;
};


Dividenden::Dividenden(double *Cdw, double *Ct, int CNr)
{
	Dw = new double[CNr];
	t  = new double[CNr];
	
	for (int i=0; i<=CNr-1; i++)
	{
		Dw[i] = Cdw[i];
		t[i]  = Ct[i];
	};
	
	Nr = CNr;
};

Dividenden::Dividenden(double D1, double t1)
{
	Nr = 1;
	Dw = new double[1];
	t = new double[1];
	Dw[0] = D1;
	t[0] = t1;
};

Dividenden::Dividenden(double D1, double t1, double D2, double t2)
{
	Nr = 2;
	Dw = new double[2];
	t = new double[2];
	Dw[0]  = D1;
	t[0]   = t1;
	Dw[1]  = D2;
	t[1]   = t2;
};



Dividenden::Dividenden(double D1, double t1, double D2, double t2, double D3, double t3)
{
	Nr = 3;
	Dw = new double[3];
	t = new double[3];
	Dw[0]  = D1;
	t[0]   = t1;
	Dw[1]  = D2;
	t[1]   = t2;
	Dw[2]  = D3;
	t[2]   = t3;
};

// Destruktor

Dividenden::~Dividenden()
{
	delete(t);
	delete(Dw);
};

// Funktionen


int Dividenden::NextDivNumber (double T)
{
	Sort();
	int NN = Nr;
	while ( (T<t[NN-1]) && (NN>0) )
	{	
		NN--;
	};
	return NN;
	
};


double Dividenden::FutureDividendValue (double  t0, double T,double  r)


{	
	double value = 0.0;
	for (int i=1; i<=Nr; i++)
	{
		if ( (t0<=t[i-1]) && (t[i-1]<=T)) value += Dw[i-1] * exp( r * (t0-t[i-1]) );
	};
	return value;
};
	
double Dividenden::Maximum (double t0, double t1, double T, double r)
{
	double value = 0.0;
	for (int i=1; i <=Nr; i++)
	{
		if ( (t0<=t[i-1]) && (t[i-1]<=t1)) 
		{
			value = max(value,FutureDividendValue(t[i-1],T,r));
		};
	};
	
	value = max(value,FutureDividendValue(t1,T,r));
	
	return value;
};

double Dividenden::FitS(double t0,double T, double r, double S)
{
	return S-FutureDividendValue(t0,T,r);
};

double Dividenden::D(int n)
{
	if ((n>0) && (n<=Nr))
	{
		return Dw[n-1];
	}
	else
	{
		return 0.0;
	};
};


double Dividenden::T(int n)
{
	if ((n>0) && (n<=Nr))
	{
		return t[n-1];
	}
	else
	{
		return 0.0;
	};
};

void Dividenden::Ausgabe()
{
	
	for (int i=0; i<=Nr-1; i++)
	{
		printf("\n T(%d) = %f    D(%d) = %f",i,t[i],i,Dw[i]);
	};
};

void Dividenden::Sort()
{
	double help;
	for (int i=0; i <=Nr-1; i++)
	{
		for(int j=i+1; j<=Nr-1; j++)
		{

			if (t[i]>t[j])
			{
				help = t[i];
				t[i] = t[j];
				t[j] = help;

				help = Dw[i];
				Dw[i] = Dw[j];
				Dw[j] = help;

			};
		};
	};

};
	

double Dividenden::Mean( int FF,
						 int LF,
						 double r,
						 double T,
						 double *TimeV,
						 double *GewV)
						 
						 //std::vector<double> &TimeV,
					     //std::vector<double> &GewV)
{
	double value = 0.0;
	double EfGew = 0.0;
	for (int i=FF; i<=LF; i++)
	{
		EfGew += GewV[i];
		for (int j=1; j<=Nr; j++)
		{
			if ( (TimeV[i]<=t[j-1]) && (t[j-1]<=T)) 
			{
				value += Dw[j-1] * exp( r * (TimeV[i]-t[j-1]) ) * GewV[i];
			};
		};
	};
	
	value /=EfGew;

	return value;
};
	

// ****************************************************************************
// *****     European - Digital Option                                    *****
// *****                                                                  *****
// *****     PayOff: Q, falls S(T)>K                                      *****
// *****                                                                  *****
// *****     Quelle: Hull                                                 *****
// ****************************************************************************

double Digital (  double S,
		    	  double K,
			      double r,
				  double d,
				  double sigma,
				  double T,
				  double Q)
{
	double d1 = (log(S/K)+(r-d-sigma*sigma/2.0)*T)/(sigma*sqrt(T));
	return (Q*exp(-r*T)*N(d1));
};




// ****************************************************************************
// *****     European - Vanilla Option                                    *****
// *****                                                                  *****
// *****     Call: max[S(T)-K,0]                                          *****
// *****     Put : max[K-S(T),0]                                          *****
// *****                                                                  *****
// *****     Quelle: Hull / Espen Gaarder Haug                            *****
// ****************************************************************************


double BlackStd ( double S,
				  double K,
				  double r,
				  double d,
				  double sigma,
				  double T,
				  OptionTyp Art)

{
	double d1,d2,value;
	
	if ( S <= 0 || r<0 || d<0 || sigma<=0.0 || T<0 ) return -999.999;
	if (T>0.0)
		{
			if (K>0) 
			{
				d1 = (log(S/K)+(r-d+sigma*sigma/2.0)*T)/(sigma*sqrt(T));
				d2 = d1-sigma*sqrt(T);
				if (Art==Call)	{value = S*exp(-d*T)*N(d1)-K*exp(-r*T)*N(d2);}
				else            {value = K*exp(-r*T)*N(-d2)-S*exp(-d*T)*N(-d1);};
			}
			else
			{
				// F�r den Fall, dass die ausstehenden Dividenden gr�sser sind als K
				
				if (Art==Call)	{value = S*exp(-d*T)-K*exp(-r*T);}
				else            {value = 0.0;};
			};
		}
	 else
		{
			  if (Art==Call) {value = max(S-K,0.0);}
			  else           {value = max(K-S,0.0);};
		};
	return value;

};


// ****************************************************************************
// *****     European - Vanilla Option mit Dividenden                     *****
// *****                                                                  *****
// *****     Call: max[S(T)-K,0]                                          *****
// *****     Put : max[K-S(T),0]                                          *****
// *****                                                                  *****
// *****     Quelle: Hull / Espen Gaarder Haug                            *****
// ****************************************************************************


double BlackStdDiv ( double S,
					 double K,
					 double r,
					 double d,
					 double sigma,
					 double T,
					 Dividenden &Div,						 	 
					 OptionTyp Art)

{
	
	double Sd = Div.FitS(0.0,T,r,S);
	// double Kd = K-Div.FutureDividendValue(T,10000.0,r);

	if ( Sd <= 0 || K<=0 || r<0 || d<0 || sigma<=0.0 || T<0 ) return -999.999;
	
	return BlackStd(Sd,K,r,d,sigma,T,Art);
};

	
double (*PAYOFF)   (double S, double K);
double (*VALUE)    (double A, double B);

double EuropeanValue (double A, double B)
{
	return A;
};

double AmericanValue (double A, double B)
{
	return max(A,B);
};


double PayoffCall (double S, double K)
{
	return (max(S-K,0.0));
};

double PayoffPut (double S, double K)
{
	return (max(K-S,0.0));
};


// ****************************************************************************
// *****     American/European - Vanilla Option mit Greeks                *****
// *****                                                                  *****
// *****     Call: max[S(tau)-K,0]                                        *****
// *****     Put : max[K-S(tau),0]                                        *****
// *****     tau in [0,T]                                                 *****
// *****                                                                  *****
// *****     Log(S)-Binomialer Baum mit Ritchken-Wahrscheinlichkeiten     *****
// *****     modifiziert mit Black-Scholes-Werten an vorletzten Knoten    *****
// *****                                                                  *****
// *****     Quelle: Peter Ritchken in The Journal of Derivatives (95)    *****
// *****     "On Pricing Barrier Options", Winter(95) S.19-S.28,          *****
// *****     Mark Broadie and Jerome Detemple at www.cirano.umontreal.ca  *****
// *****     "Recent Advances in Numerical Methods for Pricing Derivative *****
// *****      Securities " , working paper 96-s17                         *****
// ****************************************************************************



double RitchkenBinomial ( double S,
						  double K,
						  double r,
						  double d,
						  double sigma,
						  double T,
						  Dividenden &Div,
						  OptionTyp Art,
						  ExerciseTyp Exercise,
						  CalculationTyp CalcTyp,
						  int TimeSteps,
						  double &delta,
						  double &gamma,
						  double &theta)

{

	int TimeStepsT = TimeSteps+2;
	std::vector<double> Spot(2*TimeStepsT+1);
	std::vector<double> Grid(2*TimeStepsT+1);

	double S0 = Div.FitS(0.0,T,r,S);;
	double Dt;
	double dt = T/TimeSteps;
	double dx = sigma*sqrt(dt);
	double R  = exp(-r*dt);
	double mu = r - d - 0.5*sigma*sigma;
	double value,valued,help;

	double pu = R * 0.5 * ( 1.0 + mu*sqrt(dt)/sigma);
	double pd = R * 0.5 * ( 1.0 - mu*sqrt(dt)/sigma);

	int i,j;

	if (Art==Call) {PAYOFF = PayoffCall;}
	else           {PAYOFF = PayoffPut;	};

	if (Exercise==European) {VALUE = EuropeanValue;}
	else                    {VALUE = AmericanValue;};


	for (i=0; i<=2*TimeStepsT;i++)
	{
		Spot[i]    = S0*exp(double(i-TimeStepsT)*dx);
		Grid[i]    = PAYOFF(Spot[i],K);
	};

	Dt = Div.FutureDividendValue (T-dt,T,r)  ;

	if (CalcTyp==Imp)
	{
		for (j=1; j<=2*TimeStepsT-1; j=j+2)
		{
			 Grid[j] = VALUE(BlackStd(Spot[j],K,r,d,sigma,dt,Art),PAYOFF(Spot[j]+Dt,K));
		};
	}
	else
	{
		for (j=1; j<=2*TimeStepsT-1; j=j+2)
		{
			 Grid[j] = VALUE(pu*Grid[j+1]+pd*Grid[j-1],PAYOFF(Spot[j]+Dt,K));
		};
	};


	for (i=TimeStepsT-2; i>=4; i--)
	{
		Dt = Div.FutureDividendValue (double(i-2)*dt,T,r);
		for (j=TimeStepsT-i; j<=TimeStepsT+i; j=j+2)
		{
				Grid[j] = VALUE(pu*Grid[j+1]+pd*Grid[j-1],PAYOFF(Spot[j]+Dt,K));
		};
	};

	valued = Grid[TimeStepsT];

	for (i=3; i>=2; i--)
	{
		Dt = Div.FutureDividendValue (double(i-2)*dt,T,r);
		for (j=TimeStepsT-i; j<=TimeStepsT+i; j=j+2)
		{
				Grid[j] = VALUE(pu*Grid[j+1]+pd*Grid[j-1],PAYOFF(Spot[j]+Dt,K));
		};
	};

	value = Grid[TimeStepsT];

	delta  = Grid[TimeStepsT+2]-Grid[TimeStepsT-2];
	delta /=	Spot[TimeStepsT+2]-Spot[TimeStepsT-2];

	gamma  = Grid[TimeStepsT+2]-Grid[TimeStepsT];
	gamma /= Spot[TimeStepsT+2]-Spot[TimeStepsT];
	help   = Grid[TimeStepsT]-Grid[TimeStepsT-2];
	help  /= Spot[TimeStepsT]-Spot[TimeStepsT-2];
	gamma  = 2.0*(gamma-help)/(Spot[TimeStepsT+2]-Spot[TimeStepsT-2]);

	theta = ( valued - value ) / (2.0 * dt) / 365.0;

	return value;
};


// ****************************************************************************
// *****     Black-Scholes-Warrant                                        *****
// *****                                                                  *****
// *****     Call: max[S(T)-K,0]                                          *****
// *****     Put : max[K-S(T),0]                                          *****
// *****                                                                  *****
// *****     Quelle: Uwe Schulz und Siegfried Trautmann (94)              *****
// *****     "Robustness of option-like warrant valuation"                *****
// *****     Journal of Banking and Finance,18,(1994), S.841-859          *****
// ****************************************************************************


						  
double BlackScholesWarrant ( double S0,
  						     double K,
							 double r,
							 double d,
							 double sigma,
							 double T,
							 Dividenden &Div,
							 long int NShares,
							 long int NWarrants)

{
	
	if ( S0<=0 || K<=0 || r<0 || d<0  || sigma<=0 ||  T<=0 ) {return -888.888;}
	if (NShares <= 0 || NWarrants <= 0 ) { return -888.888;};
	
	const double eps = 0.0001;
	double d1,d2;

	double S  = Div.FitS(0.0,T,r,S0);;
	double ns = double(NShares);
	double nw = double(NWarrants);

	double W  = BlackStd (S,K,r,0.0,sigma,T,Call);

	double f = 0.0;
	double f1 = 1.0;

	int count = 0;
    
	// Nullstellensuche mit Newton Laufvariable: Wert des Warrants
	do
	{
		count++;
		W = W - f/f1;

		d1 = ( log((ns*S+nw*W)/(ns*K)) + (r - d + 0.5*sigma*sigma)*T ) / (sigma*sqrt(T)) ;
		d2 = d1 - sigma*sqrt(T);

		f  =  ns/(ns+nw) * ( exp(-d*T)*(ns*S+nw*W)/ns * N(d1) - K*exp(-r*T)*N(d2) ) - W;
		f1 =  nw/(ns+nw) * exp(-d*T) * N(d1) - 1.0;

	} while ((fabs(f)>eps) && (count<101)) ;

	if (count > 100)
	{
		return -100.000;
	}
	else
	{
		return W;
	};
};



int  ChangeGrid (long int n)
{
	if(fmod(double(n),2)==0) {return 0;}
	else {return 1;};
};


double BlackScholesExplizit( double S,
							 double K,
				             double r,
				             double d,
				             double sigma,
				             double T,
				             OptionTyp Art,
							 int Steps)

{

	double smaxfact = 5;
	double Smax=S*smaxfact;

	double dx = Smax/Steps;

	int OG;
    int NG = 0;

	double dt = 1/(sigma*sigma*Steps*Steps);
	dt = T/(ceil(T/dt));
	long TSteps = long (ceil(T/dt));

	double **Grid;
	Grid = new double*[2];
	Grid[0]=new double[Steps+1];
	Grid[1]=new double[Steps+1];
	
	std::vector<double> Pu(Steps+1);
	std::vector<double> Pm(Steps+1);
	std::vector<double> Pd(Steps+1);
	
	
	printf("dx:%f  dt:%f ",dx,dt);	


	int StepsU = int(ceil(ceil(Steps/smaxfact)));
	int StepsD = int(floor(ceil(Steps/smaxfact)));

	OG = ChangeGrid(TSteps);
	int j;
	int li;
	for (j=0; j<=Steps; j++)
	{
		if (Art==Call) {Grid[OG][j] = max(dx*j-K,0);}
		else           {Grid[OG][j] = max(K-dx*j,0);};

		Pu[j] = 0.5*(sigma*sigma*j*j + (r-d)*j) * dt;
		Pm[j] = 1 - (sigma*sigma*j*j*dt);
		Pd[j] = 0.5*(sigma*sigma*j*j - (r-d)*j) * dt;
	};

	
	double LowerBoundary;
	if (Art==Call)
	{
		LowerBoundary = 0.0;
	}
	else
	{
		LowerBoundary = K;
	};


	
	for (li=TSteps-1;li>=0; li--)
	{
		NG = ChangeGrid(li);
		OG = ChangeGrid(li+1);
		
		for(j=1; j<=Steps; j++)
		{
			Grid[NG][j]  = Pu[j]*Grid[OG][j+1]+Pm[j]*Grid[OG][j]+Pd[j]*Grid[OG][j-1];
			Grid[NG][j] /= (1+r*dt);
		};

		
		Grid[NG][0] = LowerBoundary;
		// Grid[NG][0] = 2*Grid[NG][Steps+1] - Grid[NG][Steps+2];
		Grid[NG][Steps] = 2*Grid[NG][Steps-1] - Grid[NG][Steps-2];

	};

	printf("\nS: %f   V:%f  ",dx*StepsU,Grid[NG][StepsU]);
	printf("\nS: %f   V:%f  ",dx*StepsD,Grid[NG][StepsD]);

	return Grid[NG][StepsU];
};
	
			


		




	

	




			
			
			

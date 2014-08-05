#include "basics.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>

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
			value = std::max(value,FutureDividendValue(t[i-1],T,r));
		};
	};

	value = std::max(value,FutureDividendValue(t1,T,r));

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

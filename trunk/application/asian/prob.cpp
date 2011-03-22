//  -------------------------------------------------------
//  -----  Includefile : prob.cpp
//  -------------------------------------------------------

//#include "StdAfx.h"
#include <math.h>
#include <stdio.h>
#include "basics.h"

extern const double Pi = 3.14159265358979323846;
extern const double InvSqrt2Pi = 1.0 / sqrt ( 2.0* Pi );
/*
#ifdef __cplusplus
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
#endif
*/


double NormalDensity ( double x )
{

	 return exp ( -x*x*0.5 )*InvSqrt2Pi;
}


double LogNormalDensity ( double x, double mu, double si )
{
	return( exp( -0.5 * pow( (log(x)-mu) / si, 2.0) ) / ( x * si * sqrt(2.0*Pi) ) );
};


double LogNormalDensity1 ( double x, double mu, double si )
{
	return( LogNormalDensity(x,mu,si) / (-1.0*x) * ( 1.0 + (log(x)-mu)/(si*si) ));
};


double LogNormalDensity2 (double x, double mu, double si )
{
	double help = pow(1.0 + (log(x)-mu) / (si*si),2.0) + ( 1.0 + (log(x)-mu) / (si*si) );
	return ( LogNormalDensity(x,mu,si) / (x*x) * ( help - 1.0 / (si*si)) );
};

double N(double x)

	{
	  const double a[ ] = {  0.2316419,	// called gamma in Hull
					 0.319381530,
					-0.356563782,
					 1.781477937,
					-1.821255978,
					 1.330274429 };
	double absx  = fabs ( x );
	double dens  = NormalDensity ( absx );
	double k     = 1.0 / ( 1.0+a[0]*absx );
	// calculate the polynomial in the Horner way
	double poly  = ((((a[5]*k + a[4])*k + a[3])*k + a[2])*k + a[1])*k;
	double ret =  ( x<0 ) ?  dens*poly : 1.0-dens*poly;
	return ret;
}

//------------------------------------------------------------------------------
//
// test report :
//
//	M. Brockmann ( 26/06/1994 )
//	NormalDensity and NormalDistribution tested against Excel 5.0
//	maximal deviation: both < 5*10^-6
//
//------------------------------------------------------------------------------

// Quantile mit Newtonverfahren

double InverseNormalDistribution ( double p )
{
	static double epsilon = 0.000001;
	static int MaxSteps = 50;
//	assert(p>0 && p<1);
	double p0 = 0.5;
	double x0 = 0.0;
	double slope;
	int steps = 0;
	while( fabs(p-p0)>epsilon && steps < MaxSteps )
	{
		slope = NormalDensity ( x0 );
		x0 -= (p0-p)/slope;
		p0 = N ( x0 );
		steps++;
	}
	if (steps==MaxSteps) {printf("Genauigkeit von InverseNormal > 0.000001");};
	return x0;
}



// replicates Sgn as in visual basic, the signum of a real number
double sgn(double a)
{
if (a>0)
        return 1.;
else if (a<0)
        return -1.;
else
        return 0.;
}


//function needed to calculate two dimensional cumulative distribution function, see Hull
double fxy(double x, double y, double a, double b, double rho)
{
    double a_s;
    double b_s;
        double result;
    a_s = a / sqrt(2 * (1 - rho * rho));
    b_s = b / sqrt(2 * (1 - rho * rho));
    result = exp(a_s * (2 * x - a_s) + b_s * (2 * y - b_s) + 2 * rho * (x - a_s) * (y - b_s));
return result;
}


// function needed to calculate two dimensional cumulative distribution function, see Hull
// this equals ND2 if a and b and rho are all nonpositive, 
// the generalization for the other cases is ND2 below
double Ntwo(double a, double b, double rho)
{
    static double aij[4]={0.325303,
                          0.4211071,
                          0.1334425,
                          0.006374323};
        static double bij[4]={0.1337764,
                          0.6243247,
                          1.3425378,
                          2.2626645};
    int i;
    int j;
        double result;
    result = 0;
        for(i=0;i<=3;i++) 
                {
                        for(j=0;j<=3;j++)
                        {
                                result+=aij[i] * aij[j] * fxy(bij[i], bij[j], a, b, rho); 
                        }
                }
    result = result * sqrt(1 - rho * rho) / Pi;
return result;
}

//calculates cumulative distribution function for a bivariate normal distribution
//see John Hull: Options, Futures and Other Derivatives
double ND2(double a, double b, double rho)
{
    double rho1;
    double rho2;
    double denominator;
        double result;

    if (rho > 0.9999)
        result = N(min(a, b));
    else if (rho < -0.9999)
        result = max(0, N(a) - N(-b));
    else
        {
        if (a * b * rho <= 0) 
                {
            if (a <= 0 && b <= 0 && rho <= 0)
                result = Ntwo(a, b, rho);
            else if (a <= 0 && b * rho >= 0)
                result = N(a) - Ntwo(a, -b, -rho);
            else if (b <= 0 && rho >= 0)
                result = N(b) - Ntwo(-a, b, -rho);
            else
                result = N(a) + N(b) - 1 + Ntwo(-a, -b, rho);
                }
        else
                {
            denominator = sqrt(a * a - 2 * rho * a * b + b * b);
            rho1 = (rho * a - b) * sgn(a) / denominator;
            rho2 = (rho * b - a) * sgn(b) / denominator;
            result = ND2(a, 0, rho1) + ND2(b, 0, rho2) - (1 - sgn(a) * sgn(b)) / 4;
        }
        if (result < 0) result = 0;
        }
        return result;
}


double Variance (double m1, double m2)
{
	return (m2- m1*m1);
};

double Skew (double m1, double m2, double m3)
{
	return (m3 - 3.0*m2*m1 + 2.0*m1*m1*m1);
};

double Kurtosis(double m1, double m2, double m3, double m4)
{
	return (m4 - 4.0*m3*m1 + 6.0*m2*m1*m1 - 3.0*pow(m1,4.0) - 3.0*Variance(m1,m2));
};


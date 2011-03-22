//  -------------------------------------------------------
//  -----  Headerfile : prob.h
//  -------------------------------------------------------

#include <math.h>


//------------------------------------------------------------------------------
// Konstants
//------------------------------------------------------------------------------

extern const double Pi;
extern const double InvSqrt2Pi;

//------------------------------------------------------------------------------
// functions
//------------------------------------------------------------------------------

// density of the standard normal distribution

extern double NormalDensity ( double x );

// density and derivations of lognormal distribution

extern double LogNormalDensity  ( double x, double mu, double si );
extern double LogNormalDensity1 ( double x, double mu, double si );
extern double LogNormalDensity2 ( double x, double mu, double si );

// cumulative distribution function of the standard normal distribution
//	precision: six decimal places
extern double N ( double x );
extern double InverseNormalDistribution ( double x);


// replicates Sgn as in visual basic, the signum of a real number
double sgn(double a);
//function needed to calculate two dimensional cumulative distribution function, 
// see Hull
double fxy(double x, double y, double a, double b, double rho);
double Ntwo(double a, double b, double rho);

//calculates cumulative distribution function for a bivariate normal distribution
//see John Hull: Options, Futures and Other Derivatives
extern double ND2(double a, double b, double rho);


// Some statistic functions

double Variance (double m1, double m2);
double Skew (double m1, double m2, double m3);
double Kurtosis(double m1, double m2, double m3, double m4);

#ifndef __RANDOM_H__
#define __RANDOM_H__
#include <stdlib.h>
#include <math.h>
#include "basics.h"


class RandomNumber
{
private:
	unsigned long RandomSeed;
	unsigned long AdditiveTerm;
	unsigned long MultiplicativeTerm;
	// modul = 2^32
	static const double Two32;
public:
// constructor
	RandomNumber ( unsigned long seed = 0L,
		       unsigned long add  = 1234567L,
		       unsigned long mult = 69069L )
		: AdditiveTerm ( add ),
		  MultiplicativeTerm ( mult )
		{ SetSeed ( seed );}
// set functions
	void SetSeed ( unsigned long seed = 345689L );
	void SetAdditiveTerm ( unsigned long add = 1234567L )
				{ AdditiveTerm=add; }
	void SetMultiplicativeTerm ( unsigned long mult = 69069L )
				{ MultiplicativeTerm=mult; }
// information functions
	unsigned long Seed ( ) { return RandomSeed; }
	unsigned long operator() ( )
				{ RandomSeed *= MultiplicativeTerm;
				  RandomSeed += AdditiveTerm;
				  return RandomSeed; }
	double Modul ( )
				{ return Two32; }
	double Uniform01 ( )
				{ return operator() ( ) / Two32; }
};

class UniformNumber : public RandomNumber
{
private:
	double Lower, Upper;
public:
// constructor
	UniformNumber ( double l = 0.0,
			double u = 1.0 )
		: Lower ( min(l,u) ), Upper ( max(l,u) )
		{ /* empty */ }
// set functions
	void SetLower ( double l = 0.0 )
				{ if( l<Upper ) Lower=l; }
	void SetUpper ( double u = 1.0 )
				{ if( u>Lower ) Upper=u; }
// information functions
	double operator() ( )
				{ return Lower + Uniform01() * (Upper-Lower); }
};

class NormalNumber : public RandomNumber
{
private:
	double Mu;
	double Variance;
	double StdDeviation;
	int SecondAvailable;
	double SecondNormal;
public:
// constructor
	NormalNumber ( double mu  = 0.0,
		       double var = 1.0 )
		: Mu ( mu ), Variance ( fabs(var) ), StdDeviation ( sqrt(fabs(var)) ), SecondAvailable ( 0 )
		{ /* empty */ }
// set functions
	void SetExpectation ( double mu = 0.0 )
				{ Mu = mu; }
	void SetVariance ( double var = 1.0 )
				{ Variance = fabs(var); StdDeviation = sqrt(Variance); }
// information functions
	double StdNormal ( );
	double operator() ( )
				{ return StdNormal()*StdDeviation+Mu; }
	double operator() ( double mu, double sigma )
				{ return StdNormal()*sigma+mu; }
};
#endif

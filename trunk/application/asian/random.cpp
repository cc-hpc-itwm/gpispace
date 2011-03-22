//#include "StdAfx.h"
#include "random.h"
#include "basics.h"
#include <math.h>
#include <time.h>

#ifdef __x86_64__
const double RandomNumber::Two32 = 18446744073709551616.0;
#else
const double RandomNumber::Two32 = 4294967296.0; 
//const double RandomNumber::Two32 = 18446744073709551616.0;
#endif

void RandomNumber::SetSeed ( unsigned long seed )
{
	if( 0==seed )
	{
		time_t timer;
		time ( &timer );
		// set RandomSeed to an odd(!) number
		RandomSeed = timer + ( timer % 2 ) - 1;
	}
	else
	{
		RandomSeed = seed;
	}
}

double NormalNumber::StdNormal ( )
{
	if( SecondAvailable )
	{
		SecondAvailable = 0;
		return SecondNormal;
	}
	double FirstUniform  = 2*Uniform01()-1;
	double SecondUniform = 2*Uniform01()-1;
	double RadiusSquared;
    do {
      FirstUniform  = 2*Uniform01()-1;
      SecondUniform = 2*Uniform01()-1;
      RadiusSquared=FirstUniform*FirstUniform+SecondUniform*SecondUniform;
    }
    while (RadiusSquared >=1.0 || 0==RadiusSquared);
#if 0
	while( RadiusSquared=FirstUniform*FirstUniform+SecondUniform*SecondUniform,
	       RadiusSquared >=1.0 || 0==RadiusSquared )
	{
        printf("radius=%f\n", RadiusSquared);

        FirstUniform  = 2*Uniform01()-1;
		SecondUniform = 2*Uniform01()-1;
	}
#endif
	double Factor = sqrt(-2.0*log(RadiusSquared)/RadiusSquared);
	SecondNormal = SecondUniform*Factor;
	SecondAvailable = 1;
	return FirstUniform*Factor;
}



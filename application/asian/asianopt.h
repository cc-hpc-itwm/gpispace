#include<vector>
#include <asian.h>

#ifndef __ASIAN_H__
#define __ASIAN_H__

enum AsianTyp {FloC,FloP,FixC,FixP};

#endif



double AsianMonteCarlo ( double &Ergebnis,
			 		     double &StdDev,
                         param_t *pstParam,
						 int LastFixing,
						 double *TimeV,
						 double *GewV,
						 AsianTyp Art,
						 bool CVBool );


bool CheckAsianParameters ( param_t *pstParam,
					        int LastFixing,
					        double *TimeV,
						 	double *GewV);

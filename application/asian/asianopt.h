#include<vector>

typedef struct param_s {
  double   m_dS;
  double   m_dK;
  double   m_dT;

  double   m_dSigma;
  double   m_dr;
  double   m_dd;
  int      m_nFirstFixing;
  double   m_dFixingsProJahr;

} param_t;

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
						 bool CVBool
                       , unsigned long number_of_rolls
                       , unsigned long seed
                       );


bool CheckAsianParameters ( param_t *pstParam,
					        int LastFixing,
					        double *TimeV,
						 	double *GewV);

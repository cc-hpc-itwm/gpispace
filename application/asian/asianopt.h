#include<vector>

#ifndef __ASIAN_H__
#define __ASIAN_H__

enum AsianTyp {FloC,FloP,FixC,FixP};

typedef struct param_s {
  double   m_dS; // Aktienpreis
  double   m_dK; // Strike
  double   m_dT; // Faelligkeit

  double   m_dSigma; // Volatilitaet
  double   m_dr; // Zinsrate
  double   m_dd;
  int      m_nFirstFixing;
  double   m_dFixingsProJahr;

  AsianTyp type;
  bool controle_variate;
} param_t;

#endif



double AsianMonteCarlo ( double &Ergebnis,
			 		     double &StdDev,
                         param_t *pstParam
                       , unsigned long number_of_rolls
                       , unsigned long seed
                       );


bool CheckAsianParameters ( param_t *pstParam,
					        int LastFixing,
					        double *TimeV,
						 	double *GewV);

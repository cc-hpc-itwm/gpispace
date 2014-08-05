#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "asianopt.h"

int main()
{

	// Programm-Parameter
  param_t stParam { 7000.0
                  , 85.0
                  , 1.02
                  , 0.2
                  , 0.05
                  , 0.0
                  , 1
                  , 50.0
                  , FixC
                  , false
                  };
  unsigned long const number_of_rolls (100000);

  double Ergebnis, StdDev;
  int LastFixing;


  long AnzahlderFixings;
  long lFixing;
  double dx;


	AnzahlderFixings = (long)(stParam.m_dFixingsProJahr * stParam.m_dT);

	LastFixing = AnzahlderFixings;

	double *TimeV = new double[LastFixing + 1];
	double *GewV = new double[LastFixing + 1];

	GewV[0] = 0.0;
	for (lFixing=1; lFixing<=AnzahlderFixings; lFixing++)
		GewV[lFixing] = 1.0;

	dx = stParam.m_dT / 365.0;

	TimeV[0] = 0.0;

	TimeV[AnzahlderFixings] = stParam.m_dT;
	for (lFixing=(AnzahlderFixings-1); lFixing>=1; lFixing--)
		TimeV[lFixing] = TimeV[lFixing + 1] - dx;

	for (lFixing=0; lFixing<=AnzahlderFixings; lFixing++)
  {
		//printf("\n%e - %e", TimeV[lFixing], GewV[lFixing]);
  }

  unsigned long seed (3134UL);

	AsianMonteCarlo (Ergebnis, StdDev,
                              &stParam,
                              LastFixing,
                              TimeV, GewV
                  , number_of_rolls
                  , seed
                  );

    printf("Sum1 = %lf\n", Ergebnis);
    printf("Sum2 = %lf\n", StdDev);

  //printf(" \n\nErgebnis = %f, Standardabweichung = %f\n", Ergebnis, StdDev);

  return 0;
}

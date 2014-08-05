#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "asianopt.h"

int main()
{

	// Programm-Parameter
  param_t stParam;

  double Ergebnis, StdDev;
  int LastFixing;


  long AnzahlderFixings;
  long lFixing;
  double dx;


  unsigned long number_of_rolls;

  // Konfigurationsdatei einlesen
  char var[512],
	line[512];

  float i;

  if (stdin) {
    while (fgets(line, sizeof(line), stdin)) {
      memset(var, 0, sizeof(var));
      sscanf (line,"%s = %f",var,&i);

      if (!strcmp (var,"S")) stParam.m_dS = (double) i;
      else if (!strcmp (var,"K")) stParam.m_dK = (double) i;
      else if (!strcmp (var,"T")) stParam.m_dT = (double) i;
      else if (!strcmp (var,"sigma")) stParam.m_dSigma = (double) i;
      else if (!strcmp (var,"r")) stParam.m_dr = (double) i;
      else if (!strcmp (var,"d")) stParam.m_dd = (double) i;
      else if (!strcmp (var,"FirstFixing")) stParam.m_nFirstFixing = (int) i;
      else if (!strcmp (var,"FixingsProJahr"))  stParam.m_dFixingsProJahr= (int) i;
      else if (!strcmp (var,"n")) number_of_rolls = (long int) i;

    }
  }

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

	stParam.type = FixC;

	stParam.controle_variate = false;

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

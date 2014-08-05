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

  unsigned long seed (3134UL);

	AsianMonteCarlo (Ergebnis, StdDev,
                              &stParam
                  , number_of_rolls
                  , seed
                  );

    printf("Sum1 = %lf\n", Ergebnis);
    printf("Sum2 = %lf\n", StdDev);

  //printf(" \n\nErgebnis = %f, Standardabweichung = %f\n", Ergebnis, StdDev);

  return 0;
}

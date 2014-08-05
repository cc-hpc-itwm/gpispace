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

	roll_result_type result (AsianMonteCarlo (
                              &stParam
                  , number_of_rolls
                  , seed
                  ));

    printf("Sum1 = %lf\n", result.sum1);
    printf("Sum2 = %lf\n", result.sum2);

//   double Wert    = exp(-pstParam->m_dr * pstParam->m_dT) * ( Sum1 / number_of_rolls );
//   //+cv*valueana*exp(r*T) ); --> ignore this part now since valueana is 0 always!
//   double Varianz = ( 1.0/number_of_rolls * ( Sum2 - 1.0/number_of_rolls * Sum1 * Sum1 ) ) / number_of_rolls;

//   Ergebnis =Wert;
//   StdDev   =sqrt(Varianz)*exp(-pstParam->m_dr*pstParam->m_dT);

  //printf(" \n\nErgebnis = %f, Standardabweichung = %f\n", Ergebnis, StdDev);

  return 0;
}

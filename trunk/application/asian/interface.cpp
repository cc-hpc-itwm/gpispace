/***********************************************************************/
/** @file interface.cpp
 *
 * $Id:$
 *
 * <short description>
 * <long description>
 *
 *  @author Kai Krueger
 *  @date   2009-02-24
 *  @email  krueger@itwm.fraunhofer.de
 *
 * (C) Fraunhofer ITWM Kaiserslautern
 **/
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * System headers
 *
 *---------------------------------------------------------------------*/
#include <iostream>

/*---------------------------------------------------------------------*
 * Local headers
 *
 *---------------------------------------------------------------------*/
#include "asianopt.h"

/*---------------------------------------------------------------------*
 * Macros
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Structures, unions, enums an d typedefs
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * File scope Variables (Variables share by several functions in
 *                       the same file )
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * External Variables
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Extern Functions declarations
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Functions declarations
 *
 *---------------------------------------------------------------------*/

void mcinterface(
  param_t stParam,
  double &Ergebnis,
  double &StdDev
)
{
  int LastFixing;
  AsianTyp Art;
  bool CVBool;

  // sonstige Parameter
  long AnzahlderFixings;
  long lDiv, lFixing;
  double dx;
  double *pDivZeit;
  double *pDivWert;
  double result;
  
  //	FixingsProJahr = 5.0;
  AnzahlderFixings = (long)(stParam.m_dFixingsProJahr * stParam.m_dT);

  //	FirstFixing = 1;
  LastFixing = AnzahlderFixings;

  double *TimeV = new double[LastFixing + 1];
  double *GewV  = new double[LastFixing + 1];

  GewV[0] = 0.0;
  for (lFixing=1; lFixing<=AnzahlderFixings; lFixing++)
    GewV[lFixing] = 1.0;

  dx = stParam.m_dT / 365.0;

  TimeV[0] = 0.0;

  TimeV[AnzahlderFixings] = stParam.m_dT;
  for (lFixing=(AnzahlderFixings-1); lFixing>=1; lFixing--)
    TimeV[lFixing] = TimeV[lFixing + 1] - dx;

  //   for (lFixing=0; lFixing<=AnzahlderFixings; lFixing++)
  //printf("\n%e - %e", TimeV[lFixing], GewV[lFixing]);

  // Dividende

  //	AnzahlderDividende = 3;
  pDivZeit = new double[stParam.m_nAnzahlderDividende + 1];
  pDivWert = new double[stParam.m_nAnzahlderDividende + 1];

  for (lDiv=0; lDiv<=stParam.m_nAnzahlderDividende; lDiv++) {
    pDivZeit[lDiv] = 0.0;
    pDivWert[lDiv] = 0.0;
  }

  Dividenden Div(pDivZeit, pDivWert, stParam.m_nAnzahlderDividende + 1);
  free(pDivZeit);
  free(pDivWert);

  // Optionstyp
  Art = FixC;

  CVBool = false;
    
  //std::cout<<"Compute MonteCarlo...."<<std::endl;

  result = AsianMonteCarlo (Ergebnis, StdDev,
                            &stParam,
                            LastFixing,
                            TimeV, GewV, Div, Art, CVBool);

  //printf("Sum1 = %lf\n", stParam.m_dSum1);
  //printf("Sum2 = %lf\n", stParam.m_dSum2);
  //printf("Sum1 = %lf\n", Ergebnis);
  //printf("Sum2 = %lf\n", StdDev);

  //printf(" \n\nErgebnis = %f, Standardabweichung = %f\n", Ergebnis, StdDev);
}

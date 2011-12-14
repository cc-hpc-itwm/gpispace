/***********************************************************************/
/** @file asian_ppu.h
 *
 * $Id:$
 *
 * <short description>
 * <long description>
 *
 *  @author Kai Krueger
 *  @date   2009-01-05
 *  @email  krueger@itwm.fraunhofer.de
 *
 * (C) Fraunhofer ITWM Kaiserslautern
 **/
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * System headers
 *
 *---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
//#ifdef __GNU__
#ifndef _MSC_VER
#  include <stdint.h>
#  include <sys/time.h>
#endif
#include <limits.h>
#include <time.h>
#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif
#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#endif

/*---------------------------------------------------------------------*
 * Local headers
 *
 *---------------------------------------------------------------------*/
//#include "spu.h"

/*---------------------------------------------------------------------*
 * Macros
 *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 * Structures, unions, enums an d typedefs
 *
 *---------------------------------------------------------------------*/
typedef struct param_s {
  double   m_dS;
  double   m_dK;
  double   m_dT;

  double   m_dSigma;
  double   m_dr;
  double   m_dd;
  int      m_nFirstFixing;
  double   m_dFixingsProJahr;
  long     m_nAnzahlderDividende;
  long int m_nn;
  double   m_dD1;   /**< modifikator von m_dS in pct */
  double   m_dD2;   /**< modifikator von m_dSigma in pct */
  
} param_t;

typedef struct result_s {
  /* Results */
  double m_dSum1;  /* mc() */
  double m_dSum2;
  double m_dDelta; /* (mc (dS + dD1) - mc(dS)) / dD1 */
  double m_dGamma; /* (mc (dS + 2*dD1) - 2*c(dS) + mc(s)) / dD1^2 */
  double m_dVega;  /* (mc (dSigma + dD2) - mc(dSigma)) / dD2 */
} result_t;

  
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
void mcinterface(param_t stParam, double &Ergebnis, double &StdDev);

/*---------------------------------------------------------------------*
 * Functions declarations
 *
 *---------------------------------------------------------------------*/


/*********************************************

   reUpdt.h
      re- mig update with teh rslt of one depth level, to be integrated in sdpa


*********************************************/

#ifndef RE_UPDT_H
#define RE_UPDT_H


#include <stdio.h>  // to write the output (allows larger files)
#include <string.h> //  memcpy(), if needed
#include <time.h>


//#include <pthread.h>

#include <signal.h>
#include <unistd.h>   // usleep -> to keep my threads sleeping


//#include "fftw3.h" //sfftw_plan_dft_1d

//---------- include some definitions used here --------------


#include "reGlbStructs.h"

/*
int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb)

*/

int reUpdate(cfg_t *pCfg, float *pCrrSlc, float *pRslt);

#endif

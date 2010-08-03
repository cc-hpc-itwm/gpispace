/*********************************************

   reProp.h
      re- mig propagate one depth level, to be integrated in sdpa


*********************************************/

#ifndef RE_PROP_H
#define RE_PROP_H


#include <stdio.h>  // to write the output (allows larger files)
#include <string.h> //  memcpy(), if needed
#include <time.h>


#include <pthread.h>

#include <signal.h>
#include <unistd.h>   // usleep -> to keep my threads sleeping


#include "fftw3.h" //sfftw_plan_dft_1d

//---------- include some definitions used here --------------

extern "C" {
  #include "/opt/cluster/Intel/ice/3.0.023/cmkl/9.0/include/mkl_types.h"
}
//#include "SegYHeader.h"

//------------------- VM -------
#include "Pv4dVM4.h"
#include "Pv4dLogger.h"


#include "reGlbStructs.h"
//#include "dmsUtls.h"


int rePropagate(cfg_t *pCfgr);

#endif

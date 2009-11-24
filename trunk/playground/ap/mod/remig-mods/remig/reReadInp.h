/*********************************************

   reReadInp.h
           routine(s) to  read & distribute the inp data


*********************************************/


#ifndef RE_READ_DAT_H
#define RE_READ_DAT_H

#include "reGlbStructs.h"

/*

int initReGlbVars(cfg_t *pCfg, TReGlbStruct *pReGlb);

int calcLoadDistribution(TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls);
int  allocAndSetLclFrqDistributionStructs(TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls, int *pIWonND);

int allocAnsSetDepthLvlsDistributionStructs(TReGlbStruct *pReG, int *izOffs, int *pIZonND);

int readAndDistributeInputData(cfg_t *pCfg, TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls);


int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb);


*/


int readAndDistributeInput(cfg_t *pCfg);





#endif

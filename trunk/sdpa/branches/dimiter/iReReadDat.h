/*********************************************

   iReReadDat.h
           routine(s) to initialize  re-, reads & distributes the inp data


*********************************************/


#ifndef INIT_RE_READ_DAT_H
#define INIT_RE_READ_DAT_H

#include "reGlbStructs.h"

/*
int readParamFile(TReGlbStruct *pReG);
int writeCheckParamFile(TReGlbStruct *pReG);

int defineAndCalcParameters(TReGlbStruct *pReG);
int printParameters(TReGlbStruct *pReG);


int initReGlbVars(cfg_t *pCfg, TReGlbStruct *pReGlb);

int calcLoadDistribution(TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls);
int  allocAndSetLclFrqDistributionStructs(TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls, int *pIWonND);

int allocAnsSetDepthLvlsDistributionStructs(TReGlbStruct *pReG, int *izOffs, int *pIZonND);

int readAndDistributeInputData(cfg_t *pCfg, TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls);


int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb);


*/

//int reApplInit(cfg_t *pCfg);

int readAndDistributeInput(cfg_t *pCfg);





#endif

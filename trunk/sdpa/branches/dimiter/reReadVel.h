/*****************************************
    reReadVel.h
     routine(s) to read vel cube & to distribute it over the nodes


*****************************************/


#ifndef RE_READ_VEL_H
#define RE_READ_VEL_H

#include "reGlbStructs.h"

/*
int calcLoadDistribution(TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls);
int  allocAndSetLclFrqDistributionStructs(TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls, int *pIWonND);

int allocAnsSetDepthLvlsDistributionStructs(TReGlbStruct *pReG, int *izOffs, int *pIZonND);


 int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb);

int convertPadAndTemperVelocityZslice( TReGlbStruct *pReG, float **vIn, float **vOut );
int readAndDistributeVelocityModel(cfg_t *pCfg, TReGlbStruct *pReGlb, int *izOffs)


*/


int readVelocity(cfg_t *pCfg);

#endif

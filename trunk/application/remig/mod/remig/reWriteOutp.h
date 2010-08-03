/*********************************************

   reWriteOutp.h
      re- trasposing the outp array and writting it


*********************************************/

#ifndef RE_WRITE_OUTP_H
#define RE_WRITE_OUTP_H



#include "reGlbStructs.h"

/*
int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb);

int allocAnsSetDepthLvlsDistributionStructs(TReGlbStruct *pReG, int *izOffs, int *pIZonND);

int cpSolnCubeIntoSharedSpace(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem, int *izOffs);

int writeOutput(TReGlbStruct *pReG, float ***pCube);
*/

int reWriteOutp(cfg_t *pCfg);


#endif

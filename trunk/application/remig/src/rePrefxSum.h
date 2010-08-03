/*********************************************

   rePrefxSum.h
      re- summing up the results after the propagation loop


*********************************************/

#ifndef RE_PRFX_SUM_H
#define RE_PRFX_SUM_H



#include "reGlbStructs.h"

/*
int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb);

int allocAnsSetDepthLvlsDistributionStructs(TReGlbStruct *pReG, int *izOffs, int *pIZonND);


tstFillInShMemCube(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem);

int tstDumpShMemSoln(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem);

int cpSolnCubeIntoSharedSpace(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem, int *izOffs);

int summUpShMemSolnCube(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem);

int ditributeShMemSolnOverTheNodes(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem, int *izOffs);
*/

int rePrefixSum(cfg_t *pCfg);

#endif


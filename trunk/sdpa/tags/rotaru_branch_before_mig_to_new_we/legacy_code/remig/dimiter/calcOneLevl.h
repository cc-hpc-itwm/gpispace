/*****************************************
    calcOneLevl.h
     routine(s) to propagate one levl downwards


*****************************************/


#ifndef RE_CALC_ONE_LEVL_H
#define RE_CALC_ONE_LEVL_H

extern "C" {
  #include "/opt/cluster/Intel/ice/3.0.023/cmkl/9.0/include/mkl_types.h"
}

#include "reGlbStructs.h"

/*
int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb);
int setPropagateExtraArrays(TReGlbStruct *pReG, float *wH, float *kx, float *kx2, 
                                     float *ky, float *ky2, float *taperx);

int get1DoffsReIm(int szX, int szY, int ix, int iy, int *offsRe, int *offsIm)

*/
// int reCalcOneLevl(cfg_t *pCfg);  // just for testing 

int reCalcOneLevl(cfg_t *pCfg, int iw, int iz, MKL_Complex8 *pSlc, float *pOutpRe);

#endif

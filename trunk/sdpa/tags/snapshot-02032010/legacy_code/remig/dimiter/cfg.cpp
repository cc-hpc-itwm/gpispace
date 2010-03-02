/***************************************
     cfg.cpp - config file, 
        the struct to be initilized  in the sdpa- init time
        and then to be used as read-only


****************************************/

#include <string.h>
#include <fvm-pc/pc.hpp> //fvmGetNodeCount()
#include <fhglog/fhglog.hpp>

#include "cfg.h"

void c_read_config(cfg_t *pSdpaInit)
{


    fvmAllocHandle_t hndGlb, hndScr;
    int rank = fvmGetRank();
    
    fvmSize_t ndSharedSz = 1000*1024*1024;
    fvmSize_t ndScratchSz =  50* 1024*1024; 
    //size_t glbVMsize = fvmGetNodeCount() * ndSharedSz;

    hndGlb =  fvmGlobalAlloc(ndSharedSz);
    hndScr = fvmGlobalAlloc(ndScratchSz);

    int szX = 100;
    int szY = 100;
    int szZ = 200;

    fvmSize_t szGlbDat = 2*sizeof(TReGlbStruct);
    fvmSize_t szInp = szX*szY*szZ*2*sizeof(float);
    fvmSize_t szVel = szX*szY*szZ*sizeof(float);
    fvmSize_t szOutp = szX*szY*szZ*sizeof(float);
    fvmSize_t szVmax = szZ*sizeof(float);

    
  for (int ndI = 0; ndI < fvmGetNodeCount(); ++ndI)
  {
    LOG(DEBUG, "initializing config for node " << ndI);
//    cfgs[node].velocity_field = velocity_field;

    pSdpaInit[ndI].nodalSharedSpaceSize = ndSharedSz;
    pSdpaInit[ndI].nodalScratchSize = ndScratchSz;

    pSdpaInit[ndI].hndGlbVMspace = hndGlb;
    pSdpaInit[ndI].hndScratch = hndScr;

    //pSdpaInit[ndI].flGlbSet = 1;
    //pSdpaInit[ndI].flShrdSet = 0;

    
    pSdpaInit[ndI].ofsGlbDat = 0;
    pSdpaInit[ndI].ofsInp = pSdpaInit[ndI].ofsGlbDat + szGlbDat;
    pSdpaInit[ndI].ofsVel = pSdpaInit[ndI].ofsInp + szInp;  
    pSdpaInit[ndI].ofsOutp =  pSdpaInit[ndI].ofsVel + szVel;
    pSdpaInit[ndI].ofsVmin = pSdpaInit[ndI].ofsOutp + szOutp;
    pSdpaInit[ndI].ofsVmax = pSdpaInit[ndI].ofsVmin + szVmax;

    pSdpaInit[ndI].ofsFree = pSdpaInit[ndI].ofsVmax + szVmax; 

    pSdpaInit[ndI].memOverflow = 0;       
    if(pSdpaInit[ndI].ofsFree > ndSharedSz) pSdpaInit[ndI].memOverflow = 1;

  }
} 

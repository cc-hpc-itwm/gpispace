/***************************************
     cfg.cpp - config file, 
        the struct to be initilized  in the sdpa- init time
        and then to be used as read-only


****************************************/

#include <string.h>
#include <fvm-pc/pc.hpp> //fvmGetNodeCount()
#include <fhglog/fhglog.hpp>

#include "cfg.h"

void c_read_config(const std::string &config_file, cfg_t *pSdpaInit)
{
    fvmAllocHandle_t hndGlb, hndScr;
    
	const std::string prefix=config_file.substr(0, config_file.find_last_of("/"));
    fvmSize_t ndSharedSz =  750 * 1024 * 1024;
    fvmSize_t ndScratchSz =  50 * 1024 * 1024; 

    hndGlb = fvmGlobalAlloc(ndSharedSz);
    hndScr = fvmGlobalAlloc(ndScratchSz);

    int szX = 100;
    int szY = 100;
    int szZ = 200;

    fvmSize_t szGlbDat = sizeof(TReGlbStruct);
    fvmSize_t szInp = szX*szY*szZ*2*sizeof(float);
    fvmSize_t szVel = szX*szY*szZ*sizeof(float);
    fvmSize_t szOutp = szX*szY*szZ*sizeof(float);
    fvmSize_t szVmax = szZ*sizeof(float);

    LOG(DEBUG, "initializing config");
	strncpy(pSdpaInit->prefix_path, prefix.c_str(), cfg_t::max_path_len);
	strncpy(pSdpaInit->config_file, config_file.c_str(), cfg_t::max_path_len);
	bzero(pSdpaInit->data_file, cfg_t::max_path_len);
	bzero(pSdpaInit->velocity_file, cfg_t::max_path_len);
	bzero(pSdpaInit->output_file, cfg_t::max_path_len);

    pSdpaInit->nodalSharedSpaceSize = ndSharedSz;
    pSdpaInit->nodalScratchSize = ndScratchSz;

    pSdpaInit->hndGlbVMspace = hndGlb;
    pSdpaInit->hndScratch = hndScr;

    pSdpaInit->ofsGlbDat = 0;
    pSdpaInit->ofsInp  = pSdpaInit->ofsGlbDat + szGlbDat;
    pSdpaInit->ofsVel  = pSdpaInit->ofsInp + szInp;  
    pSdpaInit->ofsOutp = pSdpaInit->ofsVel + szVel;
    pSdpaInit->ofsVmin = pSdpaInit->ofsOutp + szOutp;
    pSdpaInit->ofsVmax = pSdpaInit->ofsVmin + szVmax;

    pSdpaInit->ofsFree = pSdpaInit->ofsVmax + szVmax; 

    pSdpaInit->memOverflow = 0;       
    if(pSdpaInit->ofsFree > ndSharedSz) pSdpaInit->memOverflow = 1;
} 

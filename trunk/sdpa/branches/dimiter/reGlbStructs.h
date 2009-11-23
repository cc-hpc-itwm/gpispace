/*****************************************************************
    dmsGlbStruct.h 

      the global structure, defining all glb variables & mem allocations (ptrs)

******************************************************************/

#ifndef RE_GLB_STRUCT_H
#define RE_GLB_STRUCT_H 

#include <fvm/fvmAllocatorTypes.h>
#include <fvm/fvm_common.h>

#include <string.h> // size_t

extern "C" {


//--------- sdpa init struct, take the name from alex' examüple  --------
typedef struct {

    fvmSize_t nodalSharedSpaceSize; // the size of lcl-glb-hep on a node
    fvmSize_t nodalScratchSize; // the size of lcl-glb-hep on a node

    // the scratch handler 
    // scratch is the space (in the VM-heap) used by fvmGetGlobalData() & fvmPutGlobalData()
    // to exchange data between VM-heps and the glb shared space
    fvmAllocHandle_t hndScratch; // =fvmGetGlobalData(..) 

    // the handler of the lclGlbHeap allocated on ALL nodes;
    //  this is the glb shared space
    fvmAllocHandle_t hndGlbVMspace; // =fvmGetGlobalData(..)


    // int flGlbSet, flShrdSet;


    fvmShmemOffset_t ofsGlbDat;  // offs of my glb structure -> i.e. re- "global" vars   
    fvmShmemOffset_t ofsInp;     // input data offset (should i use fvmOffset_t ?? should not matter)
    fvmShmemOffset_t ofsVel;     // vel field offset
    fvmShmemOffset_t ofsOutp;    // output data offset 
    fvmShmemOffset_t ofsVmin;    // offs v-min vector 
    fvmShmemOffset_t ofsVmax;    // offs v-max vector 

    fvmShmemOffset_t ofsFree;    // offs of the free space (space not used)
  
    int memOverflow; 



} cfg_t;



//------------ reMig glb vars struct -----
typedef struct {



    //int wHlcl_startIndx, wHlcl_endIndx;  // start and end indices of the local frqs.
    //int zLevlLcl_end, zLevlLcl_start;  // you do not need these, see izOffs[]
   
    //int *pIWonND; // keeps the correspondence IW <-> ndNo, pIWonND[iw] = ndNo
    //int *pIZonND;  // keeps the correspondence IZ <-> ndNo, pIZonND[iz] = ndNo
    
        int iupd;
        int      nz,nx_in,ny_in,nt;             //!Size of model(nz,nx) and data(nt,nx)
        int      nx_ap, ny_ap;                  //!Aperture
        int      nx_out,ny_out;                 //!Size excluding aperture
	int      nx_origo,ny_origo;             //!Origo of data
	//int      nx_taper,ny_taper;             //!Size including tapers // make it local, dms
	float    dz,dx,dy,dt;                   //!Sampling

	float    fmin,fmin2,fmax2,fmax;         //!Hamming freq.parameters
	int      ntaper;                         //!length of wavefield taper 
        int max1d;

	float    df,dw;                         //!Freq sampling
    //float    w;                             //!circular freq.

	int      nwH;                           //!number omega Hamming
	int      nw,nwmin,nwmax,nwmin2,nwmax2;  //!
	    float    k0,dkx,dky;

        int  nx_fft,ny_fft,nt_fft;  // !FFT sizes


}
TReGlbStruct;

} // extern C

#endif

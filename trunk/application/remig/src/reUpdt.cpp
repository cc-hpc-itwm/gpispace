/*********************************************

   reUpdt.h
      re- mig update with teh rslt of one depth level, to be integrated in sdpa

*********************************************/


#include <fvm-pc/pc.hpp>
#include "reUpdt.h"


//-------------- cpReGlbVarsFromVM -------------
static int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb)
{

	//------ alloc shmem chunk, where to place the whole data cube ---
        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;
	
        fvmSize_t shmemLclSz = sizeof(TReGlbStruct);
        //fvmAllocHandle_t hLclShMem =fvmLocalAlloc(shmemLclSz);
        unsigned char *pShMem = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr
        bzero(pShMem, shmemLclSz);

	//--- alloc scratzch handle, for now of the same size -- 
        fvmAllocHandle_t hScra; 
        hScra = fvmLocalAlloc(shmemLclSz);

        fvmOffset_t vmOffs = pCfg->ofsGlbDat; //=0 //--- src: offs VM, see below, where the transfer occurs 
        fvmShmemOffset_t shmemOffs=0; // dest: lcl shmem offs (in the data cube)
        
        fvmSize_t transfrSZbytes=sizeof(TReGlbStruct); // bytes to transfer 
        fvmCommHandle_t commH;    // comm handle for glbPut   
        fvmCommHandleState_t commStatus;
      
        commH =  fvmGetGlobalData(hGlbVMspace,    //const fvmAllocHandle_t handle,
				  vmOffs,         //const fvmOffset_t fvmOffset,
				  transfrSZbytes, //const fvmSize_t size,
				  shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				  hScra); //const fvmAllocHandle_t scratchHandle);
        
        commStatus  = waitComm(commH);
        if(commStatus != COMM_HANDLE_OK) return (-1);

        memcpy(pReGlb, pShMem, transfrSZbytes);


       fvmLocalFree(hScra);
       //fvmLocalFree(hLclShMem); // free the local sh mem


    return 1;
}


//-------- reUpdate -----------
int reUpdate(cfg_t *pCfg, float *pCrrSlc, float *pRslt)
//
//  "reduces" (i.e. summs up)  the soln (pCrrSlc[][]) 
//  for the current iz-slice (obtained on some iz for some iw, unknown here)
//  to the result slice (for the same, but unknown iz) pRslt
//
//  the slice dimension -> nx_out, ny_out
{

   //---- the struct with re-glb vars, replicate it on each node
   TReGlbStruct *pReGlb = new TReGlbStruct;
      // copy here re-glb vars from the VM-glb space, offs == ofsGlbDat == 0; 
       cpReGlbVarsFromVM(pCfg, pReGlb);

       int sz = pReGlb->nx_out *pReGlb->ny_out;
       int i;

       for(i = 0; i < sz; i++) {
          
	   pRslt[i] += pCrrSlc[i];
 
       } 


   
    delete pReGlb; // del glbVars struct

    return 1;
}

/*********************************************

   rePrefxSum.cpp
      re- summing up the results after the propagation loop


*********************************************/

#include <stdio.h>

#include <fvm-pc/pc.hpp>
#include <sdpa/modules/util.hpp>
#include <fhglog/fhglog.hpp>

#include "utls.h"

#include "rePrefxSum.h"

//-------------- cpReGlbVarsFromVM -------------
static int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb)
{

	//------ alloc shmem chunk, where to place the whole data cube ---
        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;
	
        fvmSize_t shmemLclSz = sizeof(TReGlbStruct);
        //MR: not needed
        //fvmAllocHandle_t hLclShMem =fvmLocalAlloc(shmemLclSz);
        unsigned char *pShMem = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr
        bzero(pShMem, shmemLclSz);

	//--- alloc scratzch handle, for now of the same size -- 
        fvm::util::local_allocation hScra(shmemLclSz);

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
        
		fvm::util::wait_for_communication(commH);

        memcpy(pReGlb, pShMem, transfrSZbytes);


       //MR: not needed
       //fvmLocalFree(hLclShMem); // free the local sh mem


    return 1;
}

//--------  allocAnsSetDepthLvlsDistributionStructs -----------
//int allocAnsSetDepthLvlsDistributionStructs(TReGlbStruct *pReG, int *izOffs, int *pIZonND, int *zLevlLcl_start, int *zLevlLcl_end)
static int allocAnsSetDepthLvlsDistributionStructs(TReGlbStruct *pReG, int *izOffs, int *pIZonND)
{
    int i, j;

       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

    int sz = size;
    int base = pReG->nz/size;
    int rem = pReG->nz - base*size;
    //int *offs = new int [size];
 
    //------- alloc izOffs[] --------
    //izOffs = new int [size];
    bzero(izOffs, size*sizeof(int));

    for (i=0; i < size; i++) {
        izOffs[i] = i*base; 
    }


    for (i=1; i <= size; i++) {
        if(i <= rem) izOffs[i] += i; 
        if(i > rem) izOffs[i] += rem;         
    }


    //----- having izOffs[] one needs no zLevlLcl_start, zLevlLcl_end --
    //----- comment this later on
    //for(i=0; i < size; i++) {
//	if(rank == i) {
//	    zLevlLcl_start = izOffs[i];
//            if(i < size-1) zLevlLcl_end = izOffs[i+1]-1;
//            if(i == size-1) zLevlLcl_end = nz-1;
//        }
//    }

    //------ alloc pIZonND[] and set it-------
    //pIZonND = new int [nz];
    j=0;
    for(i = 0; i < size; i++) {	 
        if(i < size-1) {
           while(j >= izOffs[i] && (j < izOffs[i+1])) {
            pIZonND[j] = i;
            j++;
           }
        }
        if(i == size-1) {
           while(j >= izOffs[i] && (j < pReG->nz)) {
            pIZonND[j] = i;
            j++;
           }
        }
    }

    return 1;
}

//------------ tstFillInShMemCube ---------------------------------
static int tstFillInShMemCube(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem)
{
    int szZ = pReGlb->nz;
    int szX = pReGlb->nx_out;
    int szY = pReGlb->ny_out;
    int ix, iy, iz, indx;

    for (iz = 0; iz < szZ; iz++) {
	for(ix = 0; ix < szX; ix++) {
	    for(iy = 0; iy < szY; iy++) {
	         indx = indxOffsetIn3DarrayInZplaneChunks(iz, ix, iy, szZ, szX, szY);
                 ((float *) pShMem)[indx] = iz;
                 
            } //
        } // for(ix = 0; ix < szX; ix++)
    } //for (iz = 1; iz < szZ; iz++)

        


    return 1;
}

//------------ tstDumpShMemSoln ---------------------------------
static int tstDumpShMemSoln(cfg_t *pCfg, TReGlbStruct *pReG, fvmAllocHandle_t hLclShMem, unsigned char *pShMem)
{

        FILE *fp; 
        char fn[cfg_t::max_path_len];

        int i, j, iz, offs;
         
       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

        snprintf(fn, sizeof(fn), "%s/fTrace.%d.trace", pCfg->prefix_path, rank);

        if((fp=fopen(fn, "at")) != NULL) {
            //--- 
            fprintf(fp, "\n ------ printing summed-up, checking the distribution ----\n");
            //------ set to zero our shMem chunk --
            int iSz = pReG->nx_out;
            int jSz = pReG->ny_out;
            int kSz = pReG->nz;


	    float sum = 0;
                for(iz = 0; iz < kSz; iz++) { 
            
		    sum += (float) iz;     
                   for(i = 0; i < pReG->nx_out;i++) { 
                      for(j = 0; j < pReG->ny_out;j++) { //do iy=1,ny_out
                         offs = indxOffsetIn3DarrayInZplaneChunks(iz, i, j, kSz, iSz, jSz);
                         //vel = ((float*) pShMem)[offset];                       
                         fprintf(fp, "\n on iz:%d vel[ix:%d][iy:%d]:%e, (should be:%e)", 
			       iz, i, j, ((float*) pShMem)[offs], sum);
                    } //for(iy =0; iy < pReG->ny_out;iy++)
                    fprintf(fp, "\n");
                 } //for(ix =0; ix < pReG->nx_out;ix++)
              } //for(iz = 0; iz < kSz; iz++)


	    fclose(fp);
        } // if((fp=fopen(fn, "wt")) != NULL) 

    return 1;
}

//------------ cpSolnCubeIntoSharedSpace ---------------------------------
static int cpSolnCubeIntoSharedSpace(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem, int *izOffs)
{

    int  zLevlLcl_start,  zLevlLcl_end;
       int szCube = pReGlb->nz * pReGlb->nx_out *pReGlb->ny_out;

       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

       //----- get the glb-VM-space handle 
        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;

	//--- alloc scratzch handle, for now of the same size -- 
		fvm::util::local_allocation hScra(szCube*sizeof(float));

        fvmOffset_t vmOffs;     //--- src: offs VM, 
        fvmShmemOffset_t shmemOffs; // dest: lcl shmem offs (in the data cube)
        
        fvmSize_t transfrSZbytes; // bytes to transfer 
        fvmCommHandle_t commH;    // comm handle for glbPut   
        fvmCommHandleState_t commStatus;
          

        for(int iNd = 0; iNd < size; iNd++) {

	    zLevlLcl_start = izOffs[iNd];
            if(iNd < size-1) zLevlLcl_end = izOffs[iNd+1]-1;
            if(iNd == size-1) zLevlLcl_end = pReGlb->nz-1;

            transfrSZbytes = ((zLevlLcl_end+1) - zLevlLcl_start)* pReGlb->nx_out *pReGlb->ny_out*sizeof(float);

	    vmOffs = iNd * pCfg->nodalSharedSpaceSize + pCfg->ofsOutp;
            shmemOffs = izOffs[iNd] * pReGlb->nx_out *pReGlb->ny_out * sizeof(float);

            commH =  fvmGetGlobalData(hGlbVMspace,    //const fvmAllocHandle_t handle,
				  vmOffs,         //const fvmOffset_t fvmOffset,
				  transfrSZbytes, //const fvmSize_t size,
				  shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				  hScra); //const fvmAllocHandle_t scratchHandle);
        
		  fvm::util::wait_for_communication(commH);
        } // for(iNd = 0; iNd < size; iNd++)
        
    return 1;
}

//------------ summUpShMemSolnCube ---------------------------------
static int summUpShMemSolnCube(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem)
{

    int szZ = pReGlb->nz;
    int szX = pReGlb->nx_out;
    int szY = pReGlb->ny_out;
    int ix, iy, iz, indx;

    float valPrev;

    for (iz = 1; iz < szZ; iz++) {
	for(ix = 0; ix < szX; ix++) {
	    for(iy = 0; iy < szY; iy++) {
	         indx = indxOffsetIn3DarrayInZplaneChunks(iz-1, ix, iy, szZ, szX, szY);
                 valPrev = ((float *) pShMem)[indx];

	         indx = indxOffsetIn3DarrayInZplaneChunks(iz, ix, iy, szZ, szX, szY);
                 ((float *) pShMem)[indx] += valPrev;
                 
            } //
        } // for(ix = 0; ix < szX; ix++)
    } //for (iz = 1; iz < szZ; iz++)

        

    return 1;
}

//------------ ditributeShMemSolnOverTheNodes ---------------------------------
static int ditributeShMemSolnOverTheNodes(cfg_t *pCfg, TReGlbStruct *pReGlb, fvmAllocHandle_t hLclShMem, unsigned char *pShMem, int *izOffs)
{

    int  zLevlLcl_start,  zLevlLcl_end;
       int szCube = pReGlb->nz * pReGlb->nx_out *pReGlb->ny_out;

       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

       //----- get the glb-VM-space handle 
        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;

	//--- alloc scratzch handle, for now of the same size -- 
		fvm::util::local_allocation hScra(szCube*sizeof(float));

        fvmOffset_t vmOffs;     //--- dest: offs VM, 
        fvmShmemOffset_t shmemOffs; // src: lcl shmem offs (in the data cube)
        
        fvmSize_t transfrSZbytes; // bytes to transfer 
        fvmCommHandle_t commH;    // comm handle for glbPut   
        fvmCommHandleState_t commStatus;
          

        for(int iNd = 0; iNd < size; iNd++) {

	    zLevlLcl_start = izOffs[iNd];
            if(iNd < size-1) zLevlLcl_end = izOffs[iNd+1]-1;
            if(iNd == size-1) zLevlLcl_end = pReGlb->nz-1;

            transfrSZbytes = ((zLevlLcl_end+1) - zLevlLcl_start)* pReGlb->nx_out *pReGlb->ny_out*sizeof(float);

	    vmOffs = iNd * pCfg->nodalSharedSpaceSize + pCfg->ofsOutp;
            shmemOffs = izOffs[iNd] * pReGlb->nx_out *pReGlb->ny_out * sizeof(float);

            commH =  fvmPutGlobalData(hGlbVMspace,   //const fvmAllocHandle_t handle,
				     vmOffs,         //const fvmOffset_t fvmOffset,
				     transfrSZbytes, //const fvmSize_t size,
				     shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				     hScra);         //const fvmAllocHandle_t scratchHandle);
        
		  fvm::util::wait_for_communication(commH);
            
        } // for(iNd = 0; iNd < size; iNd++)

    return 1;
}




//-------------- rePrefixSum -------------
int rePrefixSum(cfg_t *pCfg)
{

   //---- the struct with re-glb vars, replicate it on each node
   TReGlbStruct *pReGlb = new TReGlbStruct;
      // copy here re-glb vars from the VM-glb space, offs == ofsGlbDat == 0; 
       cpReGlbVarsFromVM(pCfg, pReGlb);

       int szSlc = pReGlb->nx_out *pReGlb->ny_out;
       int szCube = pReGlb->nz * pReGlb->nx_out *pReGlb->ny_out;

       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

       //---- similar as for the freqs, set now depth levl distr structs ---
       int *izOffs = new int [size];
       int *pIZonND = new int [pReGlb->nz];

       allocAnsSetDepthLvlsDistributionStructs(pReGlb, izOffs, pIZonND);

       //-- 1. alloc shared mem ptr here 
	//------ alloc shmem chunk, where to place the whole soln cube ---
        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;
	
        fvmSize_t shmemLclSz = sizeof(szCube*sizeof(float));
        //MR: not needed
        //fvmAllocHandle_t hLclShMem =fvmLocalAlloc(shmemLclSz);
        // to make the signatures happy
        fvmAllocHandle_t hLclShMem = 0;
        unsigned char *pShMem = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr
        bzero(pShMem, shmemLclSz);


        //--- 2. test only -> fill in the whole soln cube with some data and distribute it over the nodes
        //tstFillInShMemCube(pCfg, pReGlb, hLclShMem, pShMem);
        //ditributeShMemSolnOverTheNodes(pCfg, pReGlb, hLclShMem, pShMem, izOffs);
        //bzero(pShMem, shmemLclSz);

        //--- 3. copy the soln cube (distributed over the nodes) into the shared space ---
        cpSolnCubeIntoSharedSpace(pCfg, pReGlb, hLclShMem, pShMem, izOffs);       

        //--- 4. summ up level by level in the soln cube, stored into the shared space --
        summUpShMemSolnCube(pCfg, pReGlb, hLclShMem, pShMem);

        //--- 5. test only -> print the summed-up soln 
        //tstDumpShMemSoln(pCfg, pReGlb, hLclShMem, pShMem); 


        //--- 6. send the summed-up soln over the nodes 
        ditributeShMemSolnOverTheNodes(pCfg, pReGlb, hLclShMem, pShMem, izOffs);

        //--- 7. test here as well ------
        //bzero(pShMem, shmemLclSz); 
        //cpSolnCubeIntoSharedSpace(pCfg, pReGlb, hLclShMem, pShMem, izOffs);       
        //tstDumpShMemSoln(pCfg, pReGlb, hLclShMem, pShMem);


        //------- free the shMem allocated here --
        // MR: not needed
        //fvmLocalFree(hLclShMem); // free the local sh mem

       //------- delete depth levl distr structs --- 
       delete [] izOffs;
       delete [] pIZonND;

    delete pReGlb; // del glbVars struct

    return 1;
}

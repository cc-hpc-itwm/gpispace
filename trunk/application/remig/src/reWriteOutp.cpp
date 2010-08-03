/*********************************************

   reWriteOutp.cpp
      re- trasposing the outp array and writting it


*********************************************/

#include <stdio.h>

#include <fvm-pc/pc.hpp>

#include "utls.h"

#include "reWriteOutp.h"


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
        fvmAllocHandle_t hScra; 
        hScra = fvmLocalAlloc(szCube*sizeof(float));

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
        
            commStatus  = waitComm(commH);
            if(commStatus != COMM_HANDLE_OK) return (-1);
            
        } // for(iNd = 0; iNd < size; iNd++)
        
	//-------- free the scratch ---
        fvmLocalFree(hScra);


    return 1;
}


//---------- writeOutput ---------------------
static int writeOutput(TReGlbStruct *pReG, float ***pCube)
{
    FILE *pfOut;
    int bytes, ix, iy, iz, pntIndx;
    int szX = pReG->nx_out;
    int szY = pReG->ny_out;
    int szZ = pReG->nz;
    

    char outfile[100];
    sprintf(outfile, "/scratch/dimiter/sdpa/Remig2000.bin");

    if((pfOut=fopen(outfile, "wb"))==NULL) {    // write at the end of the file
	//printf("\n [%d] Can not open %s", rank, outfile); 
      //exit (1);	   
    }	

    iz = 0;
//    for(ix = 0; ix < szX; ix++) {
    for(iy = 0; iy < szY; iy++) {   // NB!! do iy to be the outer loop -> to match the orig FORTRAN array settings
       for(ix = 0; ix < szX; ix++) {  
; 

//           pntIndx = indxOffsetIn3DarrayInXplaneChunks(ix, iy, iz, szX, szY, szZ);
//           bytes = fwrite((const void *) &(((float *) pCube)[pntIndx]), sizeof(float), nz, pfOut);
             bytes = fwrite((const void *) &(pCube[ix][iy][0]), sizeof(float), pReG->nz, pfOut);

        } // for(iy
    } // for(ix

   
    fclose(pfOut);

    return 1;
}


//-------------- reWriteOutp ------------
int reWriteOutp(cfg_t *pCfg)
{

   //---- the struct with re-glb vars, replicate it on each node
   TReGlbStruct *pReGlb = new TReGlbStruct;
      // copy here re-glb vars from the VM-glb space, offs == ofsGlbDat == 0; 
       cpReGlbVarsFromVM(pCfg, pReGlb);

       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

       //---- similar as for the freqs, set now depth levl distr structs ---
       int *izOffs = new int [size];
       int *pIZonND = new int [pReGlb->nz];

       allocAnsSetDepthLvlsDistributionStructs(pReGlb, izOffs, pIZonND);


       //-- 1. alloc shared mem ptr here 
	//------ alloc shmem chunk, where to place the whole soln cube ---
        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;
	
        fvmSize_t shmemLclSz = sizeof(pReGlb->nx_out*pReGlb->ny_out*pReGlb->nz*sizeof(float));

        fvmAllocHandle_t hLclShMem = 0;  // a dummy parm for  cpSolnCubeIntoSharedSpace()
        unsigned char *pShMem = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr
        bzero(pShMem, shmemLclSz);


        //--- 2. copy the soln cube (distributed over the nodes) into the shared space ---
        cpSolnCubeIntoSharedSpace(pCfg, pReGlb, hLclShMem, pShMem, izOffs);       


        //--- 3. alloc locally a 3D-cube, where to keep the transposed result
	float ***pTrnsp;
        int iSz = pReGlb->nx_out;
        int jSz = pReGlb->ny_out;
        int kSz = pReGlb->nz;
        if(utlAlloc3DfloatArrayInZplaneChunks(&pTrnsp, iSz, jSz, kSz) == 0) {
	   printf("\n [%d] Unable to allocate pUpln[][][] array \n", rank);
               // exit(0);
        } 
        utlSetToZero3DfloatArrayInZplaneChunks(pTrnsp, iSz, jSz, kSz);

        //---- 4. transpose the result cube -------------
        int ix, iy, iz, indx;

        for(ix=0; ix < iSz; ix++) {
	   for(iy=0; iy < jSz; iy++) {
              for(iz = 0; iz < kSz; iz++) {
		indx = indxOffsetIn3DarrayInZplaneChunks(iz, ix, iy, kSz, iSz, jSz);

                pTrnsp[ix][iy][iz] = ((float *) pShMem)[indx];

               } // for(iz=0
           } //for(iy=0;
        } // for(ix=0

        //---- 5. write the transposed result cube ----
	writeOutput(pReGlb, pTrnsp);

        //---- 6. free the transposed cube ----
        utlFree3DfloatArrayInZplaneChunks(pTrnsp, iSz, jSz, kSz);

       //------- delete depth levl distr structs --- 
       delete [] izOffs;
       delete [] pIZonND;

    delete pReGlb; // del glbVars struct


    return 1;
}

/*****************************************
    reReadVel.cpp
     routine(s) to read vel cube & to distribute it over the nodes


*****************************************/

#include <stdio.h>
#include <string.h>


#include <fvm-pc/pc.hpp>

//---------- include some definitions used here --------------

extern "C" {
  #include "/opt/cluster/Intel/ice/3.0.023/cmkl/9.0/include/mkl_types.h"
}

#include "utls.h"
#include "reReadVel.h"

//---------- calcLoadDistribution -------------
static int calcLoadDistribution(TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls)
// dms -> at least in the beginning:
// Keep the original fortran indexing, from 1, 
// therefore -> allocate larger (by +1) arrays.
// But in the calculations, (e.g. in this function),
// use the original fortran size, not the larger one
// Be carefull, mind also the loops !!
//
// in the VM-variant (basically) not needed, delete it later
// anyway, you should have someth. like nwHlocal[]
//
// dms, the numbr of frequencies, distributed on node #ii
// is stored in nwHlocal[ii+1]
// nwHdispls[ii+1] gives the (fortran, i.e. +1) index of the
// first frequency, ditributed on node #ii
{
    int ix;
       int rank = fvmGetRank();
       int size = fvmGetNodeCount();
    int cSize = size+1; // this is the new C-style array size

    //!**********************
    //! MPI load distribution
    //!**********************

    //! Distribute frequencies 
    //allocate(nwHlocal(1:size))
    //allocate(nwHdispls(1:(size+1)))
    
    // nwHlocal = new int [cSize];
    //nwHdispls = new int [cSize+1];
    //nwHlocal[0] = nwHdispls[0] = 0;


	int base=(pReG->nwH/size);
	int rem=pReG->nwH-size*base;
	for(ix=1; ix <= size; ix++) { //do ix=1,size
	    nwHlocal[ix]=base;
	    if(ix < rem+1) { //if (ix.lt.rem+1) then
		nwHlocal[ix] = nwHlocal[ix]+1;  //nwHlocal(ix) = nwHlocal(ix)+1
	    }//end if
	} //end do

//    } //end if(readahead)

    nwHdispls[1]=1;
    for(ix=1; ix <= size; ix++) { //do ix=1,size
	nwHdispls[ix+1]=nwHlocal[ix]+nwHdispls[ix]; //nwHdispls(ix+1)=nwHlocal(ix)+nwHdispls(ix)
    } //end do

    return 1;
}


//------------- allocAndSetLclFrqDistributionStructs ---------------
//int  allocAndSetLclFrqDistributionStructs(TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls, int *pIWonND, int *wHlcl_startIndx, *wHlcl_endIndx)
static int  allocAndSetLclFrqDistributionStructs(TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls, int *pIWonND)
// set some extra  structs managing the local freq distribution
{
    int i, j;
       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

    //------ calc the start_index and the end_index of the freqs in the FreqInpCube --
    //------ i.e. the strart & end indices of the freqs distributetd here, on this CPU --
    //int crrEndI, crrStartI = 0;
    //for(i=0; i < size; i++) {
//	crrEndI = crrStartI + nwHlocal[i+1]-1;
//        if(rank == i) {
//	    wHlcl_startIndx =crrStartI;
//            wHlcl_endIndx = crrEndI;
//        }
//        crrStartI +=  nwHlocal[i+1];
//    } 

    //------ allocv & init the array of "done" flags for each freq distributed here --
    //wHlcl_done = new int [nwH]; //nwHlocal[rank+1]];        
    //bzero(wHlcl_done, nwH*sizeof(int)); // nwHlocal[rank+1]*sizeof(int)); 


    //pv4d_printf("\n === [%d] lcl Freqs distribution -->  ttlN:%d startIndx:%d endIndx:%d ===\n", 
    //         rank, nwHlocal[rank+1], wHlcl_startIndx, wHlcl_endIndx);


    //------ alloc pIWonND[] and set it-------
    //------ dms, nwHdispls[] is fortran-indexed, its contents->also fotran indexed --
    //------ i.e. for #nd==rank one keeps in <nwHdispls[rank+1]> = <the_index_of_1st_omega_distr_on_#rank + 1>
    //pIWonND = new int [nwH];

    j=0;
    for(i = 0; i < size; i++) {	 
        if(i < size-1) {
           while((j >= nwHdispls[i+1]-1) && (j < nwHdispls[i+1 +1]-1)) {
            pIWonND[j] = i;
            j++;
           }
        }
        if(i == size-1) {
           while((j >= nwHdispls[i+1]-1) && (j < pReG->nwH)) {
            pIWonND[j] = i;
            j++;
           }
        }
    }

    //------- print the freq distribution ------
/*
    pv4d_printf("\n === [%d] lcl freq distribution -->  ttlN:%d startIndx:%d endIndx:%d ===\n", 
             rank, wHlcl_endIndx-wHlcl_startIndx+1, wHlcl_startIndx, wHlcl_endIndx);

    for(i=0; i < nwH; i++) {
	pv4d_printf("\n    [%d] freq glb ind:%d on nd:%d", rank, i, pIWonND[i]);
    } 
    pv4d_printf("\n");

    for(i=0; i < size; i++) {
       pv4d_printf("\n    [%d] nwHdispls_C[%d]:%d", rank, i+1, nwHdispls[i+1]-1);
    }
    pv4d_printf("\n");
*/

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


//------------- tuneAndTemperVelocityZslice -----------
static int  convertPadAndTemperVelocityZslice( TReGlbStruct *pReG, float **vIn, float **vOut )
{
    int ix, iy, iSz, jSz, ii, jj; 

    iSz = pReG->nx_fft; jSz = pReG->ny_fft;
    utlSetToVal2DfloatArrayC(vOut, iSz, jSz, -1);

 int feet = 0; // otherwise do it global, put it in  TReGlbStruct *pReG
    //------ shift the indices to the left --
    for(ix=0; ix < pReG->nx_in; ix++) { //do ix=1,nx_in
      for(iy=0; iy < pReG->ny_in; iy++) { //do iy=1,ny_in
         if(feet == 1) { // if(feet.eq.1)then
           //v(ix+nx_origo,iy+ny_origo)=0.3048*model(ix,iy,izSlab)/2.      //dms, model[k][j][i] -> C-indexed
           vOut[ix+pReG->nx_origo][iy+pReG->ny_origo] = 0.3048*vIn[ix][iy]/2;
         } else { //elseif(feet.eq.0)then
              if(feet == 0) 
	         //v(ix+nx_origo,iy+ny_origo)=model(ix,iy,izSlab)/2.
                 vOut[ix+pReG->nx_origo][iy+pReG->ny_origo] = vIn[ix][iy]/2;  
         } //end if
      } //end do
    } //end do


    //--- temper --
    //-- copy the part commented below and do for vOut[][] what has been done below to v[][]
    //-- But note!! vOut[][] i C-indexed, i.e always SUBTRACT ONE 

		//do iy=1+ny_origo,ny_in+ny_origo
                //    v(1:nx_origo,              iy         )=v(nx_origo+1,    iy)
                //    v(nx_in+nx_origo+1:nx_fft, iy         )=v(nx_in+nx_origo,iy)
                // end do
    for(iy=pReG->ny_origo; iy < pReG->ny_in+pReG->ny_origo; iy++) {
        //v(1:nx_origo,              iy         )=v(nx_origo+1,    iy)
        for(ii = 0; ii < pReG->nx_origo; ii++) vOut[ii][iy] = vOut[pReG->nx_origo][iy];
        //v(nx_in+nx_origo+1:nx_fft, iy         )=v(nx_in+nx_origo,iy)
        for(ii = pReG->nx_in+pReG->nx_origo; ii < pReG->nx_fft; ii++) 
                               vOut[ii][iy] = vOut[pReG->nx_in+pReG->nx_origo-1][iy];   
    } //end do


                //do ix=1+nx_origo,nx_in+nx_origo
                //    v(ix, 1:ny_origo             )=v(ix, ny_origo+1    )
                //    v(ix, ny_in+ny_origo+1:ny_fft)=v(ix, ny_in+ny_origo)
                // end do
    for(ix=pReG->nx_origo; ix < pReG->nx_in+pReG->nx_origo; ix++) {
        //v(ix, 1:ny_origo             )=v(ix, ny_origo+1    )
        for(jj = 0; jj < pReG->ny_origo; jj++) vOut[ix][jj] = vOut[ix][pReG->ny_origo];
        //v(ix, ny_in+ny_origo+1:ny_fft)=v(ix, ny_in+ny_origo)
        for(jj = pReG->ny_in+pReG->ny_origo; jj < pReG->ny_fft; jj++ ) 
                                   vOut[ix][jj]=vOut[ix][pReG->ny_in+pReG->ny_origo-1];
    } //end do

    //v(1:nx_origo,              1:ny_origo             )=v(nx_origo+1,     ny_origo+1    )
    for(ii = 0; ii < pReG->nx_origo; ii++) 
	for(jj = 0; jj < pReG->ny_origo; jj++) vOut[ii][jj] = vOut[pReG->nx_origo][pReG->ny_origo];

    //v(1:nx_origo,              ny_in+ny_origo+1:ny_fft)=v(nx_origo+1,     ny_in+ny_origo)
    for(ii = 0; ii <pReG->nx_origo; ii++) 
        for(jj = pReG->ny_in+pReG->ny_origo; jj < pReG->ny_fft; jj++) 
                  vOut[ii][jj] = vOut[pReG->nx_origo][pReG->ny_in+pReG->ny_origo-1];

    //v(nx_in+nx_origo+1:nx_fft, 1:ny_origo             )=v(nx_in+nx_origo, ny_origo+1    )
    for(ii =  pReG->nx_in+pReG->nx_origo; ii < pReG->nx_fft; ii++) 
         for(jj = 0; jj <pReG->ny_origo; jj++)
                    vOut[ii][jj] = vOut[pReG->nx_in+pReG->nx_origo-1][pReG->ny_origo];

    //v(nx_in+nx_origo+1:nx_fft, ny_in+ny_origo+1:ny_fft)=v(nx_in+nx_origo, ny_in+ny_origo)
    for(ii =  pReG->nx_in+pReG->nx_origo; ii < pReG->nx_fft; ii++) 
        for(jj = pReG->ny_in+pReG->ny_origo; jj < pReG->ny_fft; jj++)
                 vOut[ii][jj] = vOut[pReG->nx_in+pReG->nx_origo-1][pReG->ny_in+pReG->ny_origo-1];


    return 1;
}

//--------------- readAndDistributeVelocityModel ---------------
int readAndDistributeVelocityModel(cfg_t *pCfg, TReGlbStruct *pReG, int *izOffs)
{
    int i, j, iz, iSz, jSz, kSz;
    FILE *pfVel;

    float **pIZmodel;    
    float **pIZtempered;
    int bytes, offs;

       //------ use 1D-array of floats, allocate it now, the "big" cube ---
      iSz = pReG->nx_fft;
      jSz = pReG->ny_fft;
      kSz = pReG->nz; //zLevlLcl_end+1 - zLevlLcl_start; 
      float *pVelCube = new float [kSz*iSz*jSz];

        int rank = fvmGetRank();
        int size = fvmGetNodeCount();
       

      //----- the vel file ---
        const int LEN = 100;
	char velfile[LEN]; 
        //------ set here the hard-coded names (sdpa-demo only)
        strcpy(velfile, "/scratch/dimiter/sdpa/vmodGradz.bin");

    //---------- read on CPU#0 only -----
    //if(rank == 0) {
      
        if((pfVel=fopen(velfile, "rb"))==NULL) {
	     printf("\n [%d] Can not open %s", rank, velfile); 
	     //exit (1);	   
        }	


        iSz = pReG->nx_in; jSz=pReG->ny_in;
        if(utlAlloc2DfloatArray(&pIZmodel, iSz, jSz) == 0) { // if(allocstatus.ne.0) then
           printf("\n [%d] Unable to allocate pIZmodel[][] \n", rank);
                  //exit(0); //stop
        } //endif


        iSz = pReG->nx_fft; jSz=pReG->ny_fft;
        if(utlAlloc2DfloatArray(&pIZtempered, iSz, jSz) == 0) { // if(allocstatus.ne.0) then
           printf("\n [%d] Unable to allocate pIZmodel[][] \n", rank);
                  //exit(0); //stop
        } //endif

//FILE *fp; 
//char fn[100];
//sprintf(fn, "/scratch/dimiter/sdpa/fTrace%d.txt", rank);
//if((fp=fopen(fn, "at")) == NULL) {
//printf("\n [%d] can not open trace file %s \n", rank, fn);
//}

        //kSz = pReG->nz;
        //--------- read in Z-plane chunks --------- 
        for(iz=0; iz < pReG->nz; iz++) { 
          fseek(pfVel, (long int) ((iz*pReG->nx_in*pReG->ny_in)*sizeof(float)), SEEK_SET);
	    //fseek(pfVel, (long int) (((iz+1)*nx_in*ny_in)*sizeof(float)), SEEK_SET);
          bytes = fread((float *) &(pIZmodel[0][0]), sizeof(float), pReG->nx_in*pReG->ny_in, pfVel);

          convertPadAndTemperVelocityZslice(pReG, pIZmodel, pIZtempered);


          for(i = 0; i < pReG->nx_fft; i++) {
	      for(j = 0; j < pReG->ny_fft; j++) {                  
                  offs = indxOffsetIn3DarrayInZplaneChunks(iz, i, j, pReG->nz, pReG->nx_fft, pReG->ny_fft);
                  pVelCube[offs] = pIZtempered[i][j]; //pIZmodel[i][j]; //pIZtempered[i][j];
              }
          } 

//for(i = 0; i < pReG->nx_fft;i++) { 
//  for(j = 0; j < pReG->ny_fft;j++) { //do iy=1,ny_out
//     fprintf(fp, "\n testing vel temper, iz:%d vel[ix:%d][iy:%d]:%e", 
//	   iz, i, j, pIZtempered[i][j]);
//  } //for(iy =0; iy < pReG->ny_out;iy++)
//  fprintf(fp, "\n");
//} //for(ix =0; ix < pReG->nx_out;ix++)


                   
        } //for(iz=0
//fclose(fp);

        iSz = pReG->nx_in; jSz=pReG->ny_in;
        utlFree2DfloatArray(pIZmodel, iSz, jSz);

        iSz = pReG->nx_fft; jSz=pReG->ny_fft;
        utlFree2DfloatArray(pIZtempered, iSz, jSz);

        fclose(pfVel);


        //------ communicate/distribute the big cube to the vm-space on all nodes ---

        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;
	
        kSz = pReG->nz; iSz = pReG->nx_fft; jSz=pReG->ny_fft;
        fvmSize_t shmemLclSz = kSz*iSz*jSz*sizeof(float); // alloc a "big" cube, or try kSz = nz/size+1
        fvmAllocHandle_t hLclShMem =fvmLocalAlloc(shmemLclSz);
        unsigned char *pShMem = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr
        bzero(pShMem, shmemLclSz);

	//--- alloc scratzch handle, for now of the same size -- 
        fvmAllocHandle_t hScra; 
        hScra = fvmLocalAlloc(shmemLclSz);

        fvmOffset_t vmOffs; // dest: = iNd*pCfg->ndSharedSz+pCfg->ofsVel; 
        fvmShmemOffset_t shmemOffs=0; // src: lcl shmem offs 
        
        fvmSize_t transfrSZbytes; // bytes to transfer 
        fvmCommHandle_t commH;    // comm handle for glbPut   
        fvmCommHandleState_t commStatus;

        int iNd;
        int zLevlLcl_start, zLevlLcl_end;

        unsigned char *pVelChnk; 




        for(iNd=0; iNd < size; iNd++) {
	    zLevlLcl_start = izOffs[iNd];
            if(iNd < size-1) zLevlLcl_end = izOffs[iNd+1]-1;
            if(iNd == size-1) zLevlLcl_end = pReG->nz-1;

            transfrSZbytes = ((zLevlLcl_end+1) - zLevlLcl_start)*iSz*jSz*sizeof(float);

            //----- copy the right velocity chunks onto the lcl shared mem --
            pVelChnk = (unsigned char *) pVelCube + zLevlLcl_start*iSz*jSz*sizeof(float);
            memcpy(pShMem, pVelChnk, transfrSZbytes);

            //----- transfer to node #iNd 
            vmOffs= iNd*pCfg->nodalSharedSpaceSize + pCfg->ofsVel;  // the dest offs on nd #iNd
            //shmemOffs = 0;       // src offs here (presumably nd #0)
            
            commH =  fvmPutGlobalData(hGlbVMspace,   //const fvmAllocHandle_t handle,
				     vmOffs,         //const fvmOffset_t fvmOffset,
				     transfrSZbytes, //const fvmSize_t size,
				     shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				     hScra);         //const fvmAllocHandle_t scratchHandle);
	   
            commStatus  = waitComm(commH);
            if(commStatus != COMM_HANDLE_OK) return (-1);
            

        } //for(iNd=0; iNd < size; iNd++) {


        //========= find max & min vels on each depth levl ---------
        float minV, maxV, crrVel;
        float *vMin = new float [pReG->nz];
        float *vMax = new float [pReG->nz];

        int SZ=pReG->nx_fft*pReG->ny_fft;  // size of the velocity 2D array in the plane iz=const
        int startOffs;
        for(iz=0; iz < pReG->nz; iz++) {

	   startOffs = iz*SZ;
           minV = 1e30; maxV = 0;
           for(i=0; i < SZ; i++) {
              crrVel = ((float *) pVelCube)[startOffs+i];
              if(crrVel > maxV) maxV = crrVel;
              if(crrVel < minV) minV = crrVel;
           } // for(i
           if(minV < 0) printf("\n [%d] ERROR: Negative velocities iz:%d \n", rank, iz);
           vMin[iz] = minV; 
           vMax[iz] = maxV;
        } //for(iz

        //------ send vMin[] and vMax[] over the nodes ---
        //----- memcpy(pShMem, vMin[]) and memcpy(pShMem, vMax[]) shoud be put outside of the loop --
        //-----    but for now leave it like that
        for(iNd=0; iNd < size; iNd++) {
            transfrSZbytes = pReG->nz*sizeof(float);

            //----- 1st minV[], copy vMin[] onto the lcl shared mem --
            memcpy(pShMem, (unsigned char *) vMin, transfrSZbytes);

            //----- transfer minV[] to node #iNd 
            vmOffs= iNd*pCfg->nodalSharedSpaceSize + pCfg->ofsVmin;  // the dest offs on nd #iNd
            //shmemOffs = 0;       // src offs here (presumably nd #0)
            
            commH =  fvmPutGlobalData(hGlbVMspace,   //const fvmAllocHandle_t handle,
				     vmOffs,         //const fvmOffset_t fvmOffset,
				     transfrSZbytes, //const fvmSize_t size,
				     shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				     hScra);         //const fvmAllocHandle_t scratchHandle);
	   
            commStatus  = waitComm(commH);
            if(commStatus != COMM_HANDLE_OK) return (-1);
            
            //----- 2nd vMax[], copy vMax[] onto the lcl shared mem --
            memcpy(pShMem, (unsigned char *) vMax, transfrSZbytes);
            
            //----- transfer maxV[] to node #iNd 
            vmOffs= iNd*pCfg->nodalSharedSpaceSize + pCfg->ofsVmax;  // the dest offs on nd #iNd
            //shmemOffs = 0;       // src offs here (presumably nd #0)
            
            commH =  fvmPutGlobalData(hGlbVMspace,   //const fvmAllocHandle_t handle,
				     vmOffs,         //const fvmOffset_t fvmOffset,
				     transfrSZbytes, //const fvmSize_t size,
				     shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				     hScra);         //const fvmAllocHandle_t scratchHandle);
	   
            commStatus  = waitComm(commH);
            if(commStatus != COMM_HANDLE_OK) return (-1);

        } //for(iNd=0; iNd < size; iNd++) {


        delete [] vMin;
        delete [] vMax;
        //======== end max & min vels ======

        //------ now free the "big" local vel cube
        delete [] pVelCube; 

        //-------- report that inp data has been read & distributed  ---

        FILE *fp; 
        char fn[100];
        sprintf(fn, "/scratch/dimiter/sdpa/fTrace%d.txt", rank);
        if((fp=fopen(fn, "at")) != NULL) {
            //--- 
            fprintf(fp, "\n ------------------------------------------------------------------------");
            fprintf(fp, "\n        velocity read on #%d and distributed, the tests were OK      \n", rank);
            fprintf(fp, "\n        found vMin[] and vMax[] on #%d, also distributed             \n", rank);
 
            fclose(fp);
	}
	//--------- here you can do some checks, the check is OK !! ------
/*
        //FILE *fp; 
        //char fn[100];
     
        sprintf(fn, "/scratch/dimiter/sdpa/fTrace%d.txt", rank);

        if((fp=fopen(fn, "at")) != NULL) {
            //--- 
            fprintf(fp, "\n ------ printing input data, checking the distribution ----\n");
            //------ set to zero our shMem chunk --
            iSz = pReG->nx_fft;
            jSz = pReG->ny_fft;
            kSz = pReG->nz;
            //shmemLclSz = iSz*jSz*kSz *sizeof(float);
            bzero(pShMem, shmemLclSz);

            //-- in the transfer use the same vars, but they will have the opposite meaning --
            //--   -> vmOffs will be now the src offs --
            //--   -> shMemOffs is the dest offs, always == 0  (we transfer from VM-glb space to shMem)
            shmemOffs = 0;
           
            for(iNd=0; iNd < size; iNd++) {
                vmOffs= iNd*pCfg->nodalSharedSpaceSize + pCfg->ofsVel;  // the src offs on nd #iNd

	        zLevlLcl_start = izOffs[iNd];
                if(iNd < size-1) zLevlLcl_end = izOffs[iNd+1]-1;
                if(iNd == size-1) zLevlLcl_end = pReG->nz-1;
                transfrSZbytes = ((zLevlLcl_end+1) - zLevlLcl_start)*iSz*jSz*sizeof(float);
             
                commH = fvmGetGlobalData(hGlbVMspace,    //const fvmAllocHandle_t handle,
				       vmOffs,     //const fvmOffset_t fvmOffset,
				       transfrSZbytes, //const fvmSize_t size,
				       shmemOffs, //const fvmShmemOffset_t shmemOffset,
				       hScra); //const fvmAllocHandle_t scratchHandle);

                commStatus  = waitComm(commH);
                if(commStatus != COMM_HANDLE_OK) return (-1);

	        kSz = (zLevlLcl_end+1) - zLevlLcl_start;
                for(iz = 0; iz < kSz; iz++) {
                   fprintf(fp, "\n --- nz:%d, num iz-vel-slices:%d istributed to nd #%d --> iz_lcl:%d,  iz_glb:%d ----\n", 
                      pReG->nz, kSz, iNd, iz, zLevlLcl_start+iz);  
                   for(i = 0; i < pReG->nx_fft;i++) { 
                      for(j = 0; j < pReG->ny_fft;j++) { //do iy=1,ny_out
                         offs = indxOffsetIn3DarrayInZplaneChunks(iz, i, j, kSz, pReG->nx_fft, pReG->ny_fft);
                         //vel = ((float*) pShMem)[offset];                       
                         fprintf(fp, "\n on [%d], iz-vel_slices_here:%d, iz_lcl:%d, iz_glb:%d vel[ix:%d][iy:%d]:%e", 
			       iNd, kSz, iz, zLevlLcl_start+iz, i, j, ((float*) pShMem)[offs]);
                    } //for(iy =0; iy < pReG->ny_out;iy++)
                    fprintf(fp, "\n");
                 } //for(ix =0; ix < pReG->nx_out;ix++)
              } //for(iz = 0; iz < kSz; iz++)
	    } //for(iNd=0; iNd < size; iNd++) {          


	    fclose(fp);
        } // if((fp=fopen(fn, "wt")) != NULL) 
*/
        //-------- end checks, the check is OK -----------

     fvmLocalFree(hScra);
     fvmLocalFree(hLclShMem); // free the local sh mem

    return 1;
}

//-------------- cpReGlbVarsFromVM -------------
static int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb)
{

	//------ alloc shmem chunk, where to place the whole data cube ---
        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;
	
        fvmSize_t shmemLclSz = sizeof(TReGlbStruct);
        fvmAllocHandle_t hLclShMem =fvmLocalAlloc(shmemLclSz);
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
       fvmLocalFree(hLclShMem); // free the local sh mem


    return 1;
}

//----------- readVelocity ------------
int readVelocity(cfg_t *pCfg)
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

       //----- readAndDistributeVelocityModel()
       readAndDistributeVelocityModel(pCfg, pReGlb, izOffs); 

       //------- delete depth levl distr structs --- 
       delete [] izOffs;
       delete [] pIZonND;


   delete pReGlb;

    return 1;
}

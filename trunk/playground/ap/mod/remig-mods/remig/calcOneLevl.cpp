/*****************************************
    calcOneLevl.cpp
     routine(s) to propagate one levl downwards


*****************************************/


#include <stdio.h>
#include <string.h>


#include <fvm-pc/pc.hpp>
#include <sdpa/modules/util.hpp>

//---------- include some definitions used here --------------

extern "C" {
  #include "/opt/cluster/Intel/ice/3.0.023/cmkl/9.0/include/mkl_types.h"
}

#include "utls.h"

#include "propagatorsOpt.h"
#include "utilsOpt.h"

#include "calcOneLevl.h"


//-------------- cpReGlbVarsFromVM -------------
static int cpReGlbVarsFromVM(cfg_t *pCfg, TReGlbStruct *pReGlb)
{

	//------ alloc shmem chunk, where to place the whole data cube ---
        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;
	
        fvmSize_t shmemLclSz = sizeof(TReGlbStruct);
        unsigned char *pShMem = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr
        bzero(pShMem, shmemLclSz);

	//--- alloc scratzch handle, for now of the same size -- 
        fvm::util::local_allocation hScra(shmemLclSz);

        fvmOffset_t vmOffs = pCfg->ofsGlbDat; //=0 //--- src: offs VM, see below, where the transfer occurs 
        fvmShmemOffset_t shmemOffs=0; // dest: lcl shmem offs (in the data cube)
        
        fvmSize_t transfrSZbytes=sizeof(TReGlbStruct); // bytes to transfer 
        fvmCommHandle_t commH;    // comm handle for glbPut   
      
        commH =  fvmGetGlobalData(hGlbVMspace,    //const fvmAllocHandle_t handle,
				  vmOffs,         //const fvmOffset_t fvmOffset,
				  transfrSZbytes, //const fvmSize_t size,
				  shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				  hScra); //const fvmAllocHandle_t scratchHandle);
        
		fvm::util::wait_for_communication(commH);

        memcpy(pReGlb, pShMem, transfrSZbytes);

    return 1;
}



//-------------- setPropagateExtraArrays -----------
static int setPropagateExtraArrays(TReGlbStruct *pReG, float *wH, float *kx, float *kx2, 
                                         float *ky, float *ky2, float *taperx)
{

    //int iSz, jSz;

    //! set the vectors here
    //! -----------------------------------------
    //allocate(wH(1:nwH))              ! Frequency vector
    //wH = new float [pReG->nwH+1]; 
    for(int iw=1; iw <= pReG->nwH; iw++) { //do iw=1,nwH
	wH[iw]=(pReG->nwmin+iw-1)*pReG->dw; //wH(iw)=(nwmin+iw-1)*dw
    } //end do
       //allocate(kx(1:nx_fft))             ! Spatial wavenumbers
    //kx = new float [pReG->nx_fft+1];
      //allocate(kx2(1:nx_fft))            ! Spatial wavenumbers squared
    //kx2 = new float [pReG->nx_fft+1];    
      //allocate(ky(1:ny_fft))             ! Spatial wavenumbers
    //ky = new float [pReG->ny_fft+1];    
      //allocate(ky2(1:ny_fft))            ! Spatial wavenumbers squared
    //ky2 = new float pReG->[ny_fft+1];    
    
    get_kxu(pReG->nx_fft, pReG->dkx, kx, kx2);  //call get_kxu(nx_fft, dkx, kx, kx2)
    get_kxu(pReG->ny_fft, pReG->dky, ky, ky2);  //call get_kxu(ny_fft, dky, ky, ky2)
     
    
      //allocate(taperx(1:ntaper))         ! Wavefield taper
      staper(taperx,pReG->ntaper); //call staper(taperx,ntaper)
      
      for(int ix=1; ix <= pReG->ntaper; ix++) { //do ix=1,ntaper
	  taperx[ix]=taperx[ix]*taperx[ix]; // taperx(ix)=taperx(ix)*taperx(ix)
      } //end do
      //!if(rank.eq.verboseNode) print *,tapert

  return 1;
}



//-------- ger1DoffsReIm()  --------
static int get1DoffsReIm(int szX, int szY, int ix, int iy, int *offsRe, int *offsIm)
// for an item [ix][iy] in a 2D-complex array of a size [szX][szY] (x-> slowest dir),
// find the offs of the re- and the im- parts of the item
// in a 1D-memory allignment of the array
//
// i.e. the offsets (from the beginning) of the re- and the im- parts, considered  as separate floats
{
    *offsRe = 2*(ix*szY+iy);
    *offsIm = *offsRe+1;

    return 1;
}


//---------- calcOneLevl ------------
int reCalcOneLevl(cfg_t *pCfg, int iw, int iz, MKL_Complex8 *pSlc, float *pOutpRe)
//
//int reCalcOneLevl(cfg_t *pCfg)  // func def just for testing 
//
// complex pSlc[nx_fft][ny_fft] is a input/outp iw-slice, at certain depth iz, addressed in a 1D-way
//
//  as an outp this iw-slice to be used as inp for the propagation at the next depth (iz+1) levl
//
// real pOutpR[nx_out][ny_out] is the outp iz=const slice, which iz to be further reduced at depth iz 
//
// NB!! in all arrays we use 1D-layout in the mem, x-> slowest coord, y-> fastest one
//
{
/*======== defs just for testing
    int iw=5;
    int iz=0;
    MKL_Complex8 *pSlc;
    float *pOutpRe;
*****/

    const int PS=0;     // set the propagator
    int propagator = PS;

    int crrFrqI = iw;
    float w, v0, ve, k0; 

    int ix, iy, ii, jj;

   //---- the struct with re-glb vars, replicate it on each node
   TReGlbStruct reGlb;
   TReGlbStruct *pReGlb = &reGlb;
      // copy here re-glb vars from the VM-glb space, offs == ofsGlbDat == 0; 
       cpReGlbVarsFromVM(pCfg, pReGlb);
    

       //int rank = fvmGetRank();
       //int size = fvmGetNodeCount();

       //----- alloc here the aux vectors for propagation
       //----- , all of them are globals, but for now (sdpa-demo) leave them here
       float *wH = new float [pReGlb->nwH+1];  
       float *kx = new float [pReGlb->nx_fft+1];
       float *kx2 = new float [pReGlb->nx_fft+1];    
       float *ky = new float [pReGlb->ny_fft+1];    
       float *ky2 = new float [pReGlb->ny_fft+1];    
       float *taperx = new float [pReGlb->ntaper+1];

       //------ set the coeffs in the extra arrays, used in the propagators -- 
       setPropagateExtraArrays(pReGlb, wH, kx, kx2, ky, ky2, taperx);

       //--- cp vMin[] and vMax from the VM-space, first alloc the arrays -----------
       float *vMin = new float [pReGlb->nz]; 
       float *vMax  = new float [pReGlb->nz]; 


          fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;
	
          fvmSize_t shmemLclSz = pReGlb->nz * sizeof(float);
          unsigned char *pShMem = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr
          bzero(pShMem, shmemLclSz);

	  //--- alloc scratzch handle, for now of the same size -- 
          fvm::util::local_allocation hScra(shmemLclSz);

          fvmShmemOffset_t shmemOffs=0; // dest: lcl shmem offs (in the data cube)
        
          fvmSize_t transfrSZbytes= pReGlb->nz * sizeof(float); // bytes to transfer 
          fvmCommHandle_t commH;    // comm handle for glbPut   

          //-------- vMin[] -----      
          fvmOffset_t vmOffs = pCfg->ofsVmin; //--- src: offs VM, see below, where the transfer occurs 
          commH =  fvmGetGlobalData(hGlbVMspace,    //const fvmAllocHandle_t handle,
				  vmOffs,         //const fvmOffset_t fvmOffset,
				  transfrSZbytes, //const fvmSize_t size,
				  shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				  hScra); //const fvmAllocHandle_t scratchHandle);
        
		  fvm::util::wait_for_communication(commH);

          memcpy(vMin, pShMem, transfrSZbytes);

          //-------- vMax[] -----      
          vmOffs = pCfg->ofsVmax; //--- src: offs VM, see below, where the transfer occurs 
          commH =  fvmGetGlobalData(hGlbVMspace,    //const fvmAllocHandle_t handle,
				  vmOffs,         //const fvmOffset_t fvmOffset,
				  transfrSZbytes, //const fvmSize_t size,
				  shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				  hScra); //const fvmAllocHandle_t scratchHandle);
        
		  fvm::util::wait_for_communication(commH);

          memcpy(vMax, pShMem, transfrSZbytes);


       //------- alloc the work-array for the FFT ---
       //float* const       u1;       // temp space 4*roundUp4(nx) complex
       //u1 = new float const [pReGlb->nx_fft];
       float *u1 = new float [2*4*roundUp4(pReGlb->nx_fft)];  // dms, roundUp4() -> martin, 2* -> complex !
       MKL_Complex8 *pWrk = new MKL_Complex8 [roundUp4(pReGlb->nx_fft) * roundUp4(pReGlb->ny_fft)];


       //------ copy the data to the blocked wrk array  -------
	  matrixToBlockedFloat4( (float*) pSlc, (float *)pWrk, pReGlb->nx_fft, 2*pReGlb->ny_fft, 2*roundUp4(pReGlb->ny_fft) );


//=== to test: transfer iw-slice data from VM, the test passed OK ---
/*
    transfrSZbytes= pReGlb->nx_fft * pReGlb->ny_fft * sizeof(MKL_Complex8);
    pSlc = new MKL_Complex8 [pReGlb->nx_fft * pReGlb->ny_fft];
    pOutpRe = new float [pReGlb->nx_out * pReGlb->ny_out];


    hGlbVMspace = pCfg->hndGlbVMspace;	
    shmemLclSz = transfrSZbytes;
    pShMem = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr
    bzero(pShMem, shmemLclSz);

       //--- alloc scratzch handle, for now of the same size -- 
    fvm::util::local_allocation hScra(shmemLclSz);

    shmemOffs=0; // dest: lcl shmem offs (in the data cube)
        
        //-------- offsInp data -----      
    vmOffs = pCfg->ofsInp + iw*transfrSZbytes; //--- src: offs VM, see below, where the transfer occurs 
    commH =  fvmGetGlobalData(hGlbVMspace,    //const fvmAllocHandle_t handle,
				  vmOffs,         //const fvmOffset_t fvmOffset,
				  transfrSZbytes, //const fvmSize_t size,
				  shmemOffs,      //const fvmShmemOffset_t shmemOffset,
				  hScra); //const fvmAllocHandle_t scratchHandle);
        
    commStatus  = waitComm(commH);
    if(commStatus != COMM_HANDLE_OK) return (-1);

    //memcpy(pSlc, pShMem, transfrSZbytes);
    matrixToBlockedFloat4( (float*) pShMem, (float *)pWrk, pReGlb->nx_fft, 2*pReGlb->ny_fft, 2*roundUp4(pReGlb->ny_fft) );  
       
    fvmLocalFree(hScra);
    //fvmLocalFree(hLclShMem); // free the local sh mem
*/
//=== end test-section, transfr iw-slice from VM. the test passed OK === 


      //--- start now the downwards propagation --------
       w = wH[crrFrqI+1];

       v0 = vMin[iz];  // min vel
       ve = vMax[iz];  // max vel 


 
//=== start test-section, print dat before propagate, the test passed OK ===
/*
       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

        FILE *fp; 
        char fn[100];
     sprintf(fn, "/scratch/dimiter/sdpa/fTrace%d.txt", rank);

     blockedToMatrixFloat4( (float*)pWrk , (float *)pSlc, pReGlb->nx_fft, 2*pReGlb->ny_fft, 2*roundUp4(pReGlb->ny_fft) );  
    
     if((fp=fopen(fn, "at")) != NULL) {
         
	 fprintf(fp,"\n\n ------- the data array before propagator ---------\n");

         for(ix=0; ix < pReGlb->nx_fft; ix++) {
	     for(iy = 0; iy < pReGlb->ny_fft; iy++) {
                 ii = ix*pReGlb->ny_fft+iy;
		 fprintf(fp, "\n befr prop iz:%d iw:%d w:%e v_min:%e v_max:%e u[ix:%d][iy:%d] -> re:%e im:%e",
                         iz, crrFrqI, w, v0, ve, ix, iy, pSlc[ii].real, pSlc[ii].imag); 
             } //for(iy = 0; iy < ny_fft; iy++) 
             fprintf(fp, "\n");
         } // for(ix=0; ix < pCfg->nx_fft; ix++)

	 fclose(fp);

     } // if((fp=fopen(fn, "at")) != NULL)
*/
//=== end test-section, dat before propagat, the test passed OK ==========


       k0=w/(v0);                             //k0=w/(v0) // v0->min velocity

	    if( (ve-v0)/v0 > 0.01) {   //if((ve-v0)/v0.gt.0.01) then
	       if(propagator == PS) {    //if (propagator.eq.PS) then
		    //call PO3D_PS(u, nx_fft, ny_fft, kx2, ky2, k0, dz, &
                             //  &iupd, plan_w1, plan_w2 )
		   //pv4d_printf("\n    [%d] thread:%d iz:%d iw:%d calling PS-propagator",pThisCls->rank, pPrm->threadIndx, iz, crrFrqI);
		   //PO3D_PS(pPrm->pU, pThisCls->nx_fft, pThisCls->ny_fft, pThisCls->kx2, pThisCls->ky2, 
                   //                   k0, pThisCls->dz, pThisCls->iupd, *(pPrm->pPlan1), *(pPrm->pPlan2));

                   po3d_ps_opt(
		       (float *) pWrk,   // float* const       u,        // complex u(nx, ny) frequency slice of wave field
                       pReGlb->nx_fft,   // const int          nx,       //! number of grids in x-direction
                       pReGlb->ny_fft,   // const int          ny,       //! number of grids in y-direction
                       kx2,              // const float* const kx2,      // kx2(nx) square of wave-number array in x-direction
                       ky2,              // const float* const ky2,      // ky2(ny) square of wave-number array in y-direction
                       k0,               // const float        k0,       //! reference wave-number
                       pReGlb->dz,       // const float        dz,       //! depth interval
                       u1,               // float* const       u1,       // temp space 4*roundUp4(nx) complex
                       pReGlb->iupd      // const int          iupd      //! up/down flag: -1=forward, +1=backword
                       );


               } //end if(propagator == PS) {

/*
               if(pThisCls->propagator == SSF) { //if (propagator.eq.SSF) then
		    //call PO3D_SSF(u, nx_fft, ny_fft, kx2, ky2, v, v0, dz, w, &
		    // &iupd, plan_w1, plan_w2 )
               } //end if
               if(pThisCls->propagator == GSP1) { //if (propagator.eq.GSP1) then
		    //call PO3D_GSP1(u, nx_fft, ny_fft, kx2, ky2, v, v0, dz, w, &
                    //   &dx, dy, u1, v1, za, zb, zc, zd, ze, max1d, &
                    //    &iupd, plan_w1, plan_w2 )
               } //end if
               if(pThisCls->propagator == GSP2) { //if (propagator.eq.GSP2) then
                    //call PO3D_GSP2(u, nx_fft, ny_fft, kx2, ky2, v, v0, dz, w, &
                    //      &dx, dy, u1, v1, za, zb, zc, zd, ze, max1d, &
                    //      &a1, b1, a2, iupd, plan_w1, plan_w2 )
               } //end if
*/

            }
            else {
               //PO3D_PS(u, nx_fft, ny_fft, kx2, ky2, k0, dz, iupd, plan_w1, plan_w2);
		//pv4d_printf("\n    [%d] thread:%d iz:%d iw:%d calling PS-propagator",pThisCls->rank, pPrm->threadIndx, iz, crrFrqI);
		//PO3D_PS(pPrm->pU, pThisCls->nx_fft, pThisCls->ny_fft, pThisCls->kx2, pThisCls->ky2, 
                //                      k0, pThisCls->dz, pThisCls->iupd, *(pPrm->pPlan1), *(pPrm->pPlan2));
                   //                   k0, pThisCls->dz, pThisCls->iupd, *(pPrm->pPlan1), *(pPrm->pPlan2));


                   po3d_ps_opt(
		       (float *) pWrk,   // float* const       u,        // complex u(nx, ny) frequency slice of wave field
                       pReGlb->nx_fft,   // const int          nx,       //! number of grids in x-direction
                       pReGlb->ny_fft,   // const int          ny,       //! number of grids in y-direction
                       kx2,              // const float* const kx2,      // kx2(nx) square of wave-number array in x-direction
                       ky2,              // const float* const ky2,      // ky2(ny) square of wave-number array in y-direction
                       k0,               // const float        k0,       //! reference wave-number
                       pReGlb->dz,       // const float        dz,       //! depth interval
                       u1,               // float* const       u1,       // temp space 4*roundUp4(nx) complex
                       pReGlb->iupd      // const int          iupd      //! up/down flag: -1=forward, +1=backword
                       );

            } //endiff( (ve-v0)/v0 > 0.01) 

	    blockedToMatrixFloat4( (float*) pWrk, (float*) pSlc, pReGlb->nx_fft, 2*pReGlb->ny_fft, 2*roundUp4(pReGlb->ny_fft) );

//=== start test-section, dat after propagate, the test passed OK ==
//	{
//
//     int rank = fvmGetRank();
//     int size = fvmGetNodeCount();
//     FILE *fp; 
//     char fn[100];
//     sprintf(fn, "/scratch/petry/sdpa/fTrace%d.txt", rank);
//
//     if((fp=fopen(fn, "at")) != NULL) {
//         
//	 fprintf(fp,"\n\n ------- the data array AFTER propagator ---------\n");
//
//         for(ix=0; ix < pReGlb->nx_fft; ix++) {
//	     for(iy = 0; iy < pReGlb->ny_fft; iy++) {
//                 ii = ix*pReGlb->ny_fft+iy;
//		 fprintf(fp, "\n AFTR prop iz:%d iw:%d w:%e v_min:%e v_max:%e u[ix:%d][iy:%d] -> re:%e im:%e",
//                         iz, crrFrqI, w, v0, ve, ix, iy, pSlc[ii].real, pSlc[ii].imag); 
//             } //for(iy = 0; iy < ny_fft; iy++) 
//             fprintf(fp, "\n");
//         } // for(ix=0; ix < pCfg->nx_fft; ix++)
//
//	 fclose(fp);
//
//     } // if((fp=fopen(fn, "at")) != NULL)
//	}

//=== end test-section, dat aftr propagate, the test passed OK ===


	    //------ taper data -----------
              //------ u_tapered array, allocate ---
             MKL_Complex8 **uTaper;
            int iSz = pReGlb->nx_fft;  
            int jSz = pReGlb->ny_fft; // u[i][j] -> C-indexing, do not invert indices, j ->faster one 
            if(utlAlloc2DcomplArray(&uTaper, iSz, jSz) == 0) {
	         printf("\n calcOneLevl: -> Unable to allocate matrix uTaper[][] \n");
                 //exit(0); //stop
				 return -1;
            } //end if


	   //------ tapering, here keep the original code, just modify it (replace foutput[][][] by uTaper[][])
           //------- send the data back to uTaper[][] 
            transfrSZbytes = pReGlb->nx_fft * pReGlb->ny_fft * 2*sizeof(float);
	    memcpy((unsigned char *) &(uTaper[0][0]), (unsigned char *) pSlc, transfrSZbytes);



            //! Taper data
            int nx_taper = pReGlb->nx_out + 2*pReGlb->ntaper;
            int ny_taper = pReGlb->ny_out + 2*pReGlb->ntaper;

            for(iy = 1+pReGlb->ntaper; iy <= pReGlb->ny_out + pReGlb->ntaper; iy++) { // do iy=1+ntaper,ny_out+ntaper
	      for(ix = 1; ix <= pReGlb->ntaper; ix++) { //do ix=1,ntaper
		 //foutput3(iw,ix,           iy)=foutput3(iw,ix,           iy)*taperx(ix)
		 uTaper[ix-1][iy-1].real *= taperx[ix];
 		 uTaper[ix-1][iy-1].imag *= taperx[ix];

                 //foutput3(iw,nx_taper-ix+1,iy)=foutput3(iw,nx_taper-ix+1,iy)*taperx(ix)
                 uTaper[nx_taper-ix+1-1][iy-1].real *= taperx[ix];
                 uTaper[nx_taper-ix+1-1][iy-1].imag *= taperx[ix];
              } //enddo
            } //enddo

            for(iy=1; iy <= pReGlb->ntaper; iy++) { //do iy=1,ntaper
	      for(ix = 1 + pReGlb->ntaper; ix <= pReGlb->nx_out+pReGlb->ntaper; ix++) {  //do ix=1+ntaper,nx_out+ntaper
		 //foutput3(iw,ix,iy           )=foutput3(iw,ix,iy           )*taperx(iy)
                 uTaper[ix-1][iy-1].real  *= taperx[iy];
                 uTaper[ix-1][iy-1].imag  *= taperx[iy];

                 //foutput3(iw,ix,ny_taper-iy+1)=foutput3(iw,ix,ny_taper-iy+1)*taperx(iy)
                 uTaper[ix-1][ny_taper-iy+1-1].real *= taperx[iy];
                 uTaper[ix-1][ny_taper-iy+1-1].imag *= taperx[iy];

               } //enddo
             } //enddo


	    float tmpVar;
             for(iy = 1; iy <= pReGlb->ntaper; iy++) { //do iy=1,ntaper
	       for(ix = 1; ix <= pReGlb->ntaper; ix++) { //do ix=1,ntaper

		  tmpVar = taperx[ix] * taperx[iy];

		  //foutput3(iw,ix,           iy           )=foutput3(iw,ix,           iy           )*taperx(ix)*taperx(iy)
		  uTaper[ix-1][iy-1].real *= tmpVar; 
                  uTaper[ix-1][iy-1].imag *= tmpVar;

                  //foutput3(iw,nx_taper-ix+1,iy           )=foutput3(iw,nx_taper-ix+1,iy           )*taperx(ix)*taperx(iy)
                  uTaper[nx_taper-ix+1-1][iy-1].real *= tmpVar; 
                  uTaper[nx_taper-ix+1-1][iy-1].imag *= tmpVar; 

                  //foutput3(iw,ix,           ny_taper-iy+1)=foutput3(iw,ix,           ny_taper-iy+1)*taperx(ix)*taperx(iy)
                  uTaper[ix-1][ny_taper-iy+1-1].real *= tmpVar; 
                  uTaper[ix-1][ny_taper-iy+1-1].imag *= tmpVar; 

                  //foutput3(iw,nx_taper-ix+1,ny_taper-iy+1)=foutput3(iw,nx_taper-ix+1,ny_taper-iy+1)*taperx(ix)*taperx(iy)
                  uTaper[nx_taper-ix+1-1][ny_taper-iy+1-1].real *= tmpVar;
                  uTaper[nx_taper-ix+1-1][ny_taper-iy+1-1].imag *= tmpVar;

	       } //end do
             } //end do


             //foutput3(iw,nx_taper+1:nx_fft,          1:ny_fft)=cmplx(0.0,0.0)
             for(ii = nx_taper+1; ii <= pReGlb->nx_fft; ii++) {
	       for(jj  = 1; jj <= pReGlb->ny_fft; jj++) {
			    uTaper[ii-1][jj-1].real = 0;
			    uTaper[ii-1][jj-1].imag = 0;
	       } // for jj
	     } // for(ii

             //foutput3(iw,1:nx_fft,          ny_taper+1:ny_fft)=cmplx(0.0,0.0)
             for(ii = 1; ii <= pReGlb->nx_fft; ii++) {
	       for(jj  = ny_taper+1; jj <= pReGlb->ny_fft; jj++) {
		  uTaper[ii-1][jj-1].real = 0;
		  uTaper[ii-1][jj-1].imag = 0;
	       } // for jj
	     } // for(ii


             int offsOutp;
             //------- cutt off the origo extra part of the domain  ---
	     for(ix = 1; ix <= pReGlb->nx_in; ix++) { //do ix=1,nx_in
                for(iy = 1; iy <= pReGlb->ny_in; iy++)  { // do iy=1,ny_in
			//migrated(ix,iy,izSlab)=migrated(ix,iy,izSlab)+&
			//&real(sum(foutput3(1:nwHlocal(rank+1),nx_origo+ix,ny_origo+iy)))
			
                        // migrChunk[ix-1][iy-1] += uTaper[pReGlb->nx_origo+ix-1][pReGlb->ny_origo+iy-1].real;
                        // dms, now replace "+=" by "=", the reduction is to be done by thread #3 on the right node
                        // also the summation level by level afterwards
		    // migrChunk[ix-1][iy-1] = uTaper[pReGlb->nx_origo+ix-1][pReGlb->ny_origo+iy-1].real;
                        // migrChunk[ix-1][iy-1] += uTaper[pReGlb->nx_origo+ix-1][pReGlb->ny_origo+iy-1].real;

                    offsOutp = (ix-1)*pReGlb->ny_in + (iy-1);
                    pOutpRe[offsOutp] = uTaper[pReGlb->nx_origo+ix-1][pReGlb->ny_origo+iy-1].real;
		} 
	     } 


	     //-------- cp the tapered array to the output ----------
            transfrSZbytes = pReGlb->nx_fft * pReGlb->ny_fft * 2*sizeof(float);
	    memcpy((char *) pSlc, (char *) &(uTaper[0][0]), transfrSZbytes);

            iSz = pReGlb->nx_fft;  
            jSz = pReGlb->ny_fft; // u[i][j] -> C-indexing, do not invert indices, j ->faster one 
            utlFree2DcomplArray(uTaper, iSz, jSz);

//=== start test-section, dat after temper, the test passed OK ==
// {
// 
//     int rank = fvmGetRank();
//     int size = fvmGetNodeCount();
//     FILE *fp; 
//     char fn[100];
//     sprintf(fn, "/scratch/petry/sdpa/fTrace%d.txt", rank);
//
//     if((fp=fopen(fn, "at")) != NULL) {
//         
//	 fprintf(fp,"\n\n ------- the data array AFTER temper ---------\n");
//
//         for(ix=0; ix < pReGlb->nx_out; ix++) {
//	     for(iy = 0; iy < pReGlb->ny_out; iy++) {
//                 ii = ix*pReGlb->ny_out+iy;
//		 fprintf(fp, "\n aftr temper iz:%d iw:%d w:%e v_min:%e v_max:%e pOutpRe[ix:%d][iy:%d]:%e ",
//                         iz, crrFrqI, w, v0, ve, ix, iy, pOutpRe[ii]); 
//             } //for(iy = 0; iy < ny_fft; iy++) 
//             fprintf(fp, "\n");
//         } // for(ix=0; ix < pCfg->nx_fft; ix++)
//
//	 fclose(fp);
//
//     } // if((fp=fopen(fn, "at")) != NULL)
// }

//=== end test-section, dat aftr propagate, the test passed OK ===

//	{
//
//     int rank = fvmGetRank();
//     int size = fvmGetNodeCount();
//     FILE *fp; 
//     char fn[100];
//     sprintf(fn, "/scratch/petry/sdpa/fTrace%d.txt", rank);
//
//     if((fp=fopen(fn, "at")) != NULL) {
//         
//	 fprintf(fp,"\n\n ------- the input array AFTER temper ---------\n");
//
//         for(ix=0; ix < pReGlb->nx_fft; ix++) {
//	     for(iy = 0; iy < pReGlb->ny_fft; iy++) {
//                 ii = ix*pReGlb->ny_fft+iy;
//		 fprintf(fp, "\n AFTR temper iz:%d iw:%d w:%e v_min:%e v_max:%e u[ix:%d][iy:%d] -> re:%e im:%e",
//                         iz, crrFrqI, w, v0, ve, ix, iy, pSlc[ii].real, pSlc[ii].imag); 
//             } //for(iy = 0; iy < ny_fft; iy++) 
//             fprintf(fp, "\n");
//         } // for(ix=0; ix < pCfg->nx_fft; ix++)
//
//	 fclose(fp);
//
//     } // if((fp=fopen(fn, "at")) != NULL)
//	}


//=== to test: delete locally allocated arrays ---
/*
    delete [] pSlc;
    delete [] pOutpRe; 
*/
//==== to test:delete locally allocated arrays ---

       //------- delete the work-array for the FFT ---
       delete [] pWrk;
       delete [] u1;


       //--- free vMin[] and vMax[] -------
       delete [] vMax;
       delete [] vMin;

       //---- delete here the aux vectors, which are - otherwise - global --
       delete [] taperx;
       delete [] ky2;
       delete [] ky;
       delete [] kx2;
       delete [] kx;
       delete [] wH;

    return 1;
}



















/*********  do not delete this, use it later on !!!
            // do not forget to reduce the indices by -1 when you call get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);

            //! Taper data
            int nx_taper = pReGlb->nx_out + 2*pReGlb->ntaper;
            int ny_taper = pReGlb->ny_out + 2*pReGlb->ntaper;

            int szX = pReGlb->nx_out;
            int szY = pReGlb->ny_out;
            int offsRe, offsIm;
	    //get1DoffsReIm(int szX, int szY, int ix, int iy, int *offsRe, int *offsIm)


            for(iy = 1+pReGlb->ntaper; iy <= pReGlb->ny_out + pReGlb->ntaper; iy++) { // do iy=1+ntaper,ny_out+ntaper
	      for(ix = 1; ix <= pReGlb->ntaper; ix++) { //do ix=1,ntaper
		 //foutput3(iw,ix,           iy)=foutput3(iw,ix,           iy)*taperx(ix)
		 //pSlc[ix-1][iy-1].real *= taperx[ix];
 		 //pSlc[ix-1][iy-1].imag *= taperx[ix];

		 ii=ix-1; jj = iy-1;
		 get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);
                 ((float *)pSlc)[offsRe] *= taperx[ix];
                 ((float *)pSlc)[offsIm] *= taperx[ix];
                 

                 //foutput3(iw,nx_taper-ix+1,iy)=foutput3(iw,nx_taper-ix+1,iy)*taperx(ix)
                 //pSlc[nx_taper-ix+1-1][iy-1].real *= pReGlb->taperx[ix];
                 //pSlc[nx_taper-ix+1-1][iy-1].imag *= pReGlb->taperx[ix];
		 ii=nx_taper-ix+1-1; jj = iy-1;
		 get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);
                 ((float *)pSlc)[offsRe] *= taperx[ix];
                 ((float *)pSlc)[offsIm] *= taperx[ix];
 
              } //enddo
            } //enddo

            for(iy=1; iy <= pReGlb->ntaper; iy++) { //do iy=1,ntaper
	      for(ix = 1 + pReGlb->ntaper; ix <= pReGlb->nx_out+pReGlb->ntaper; ix++) {  //do ix=1+ntaper,nx_out+ntaper
		 //foutput3(iw,ix,iy           )=foutput3(iw,ix,iy           )*taperx(iy)
                 //pSlc[ix-1][iy-1].real  *= pReGlb->taperx[iy];
                 //pSlc[ix-1][iy-1].imag  *= pReGlb->taperx[iy];
		 ii=ix-1; jj = iy-1;
		 get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);
                 ((float *)pSlc)[offsRe] *= taperx[iy];
                 ((float *)pSlc)[offsIm] *= taperx[iy];


                 //foutput3(iw,ix,ny_taper-iy+1)=foutput3(iw,ix,ny_taper-iy+1)*taperx(iy)
                 //pSlc[ix-1][ny_taper-iy+1-1].real *= pReGlb->taperx[iy];
                 //pSlc[ix-1][ny_taper-iy+1-1].imag *= pReGlb->taperx[iy];
		 ii=ix-1; jj = ny_taper-iy+1-1;
		 get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);
                 ((float *)pSlc)[offsRe] *= taperx[iy];
                 ((float *)pSlc)[offsIm] *= taperx[iy];

               } //enddo
             } //enddo

	    float tmpVar;
             for(iy = 1; iy <= pReGlb->ntaper; iy++) { //do iy=1,ntaper
	       for(ix = 1; ix <= pReGlb->ntaper; ix++) { //do ix=1,ntaper

		  tmpVar = taperx[ix] * taperx[iy];

		  //foutput3(iw,ix,           iy           )=foutput3(iw,ix,           iy           )*taperx(ix)*taperx(iy)
		  //pSlc[ix-1][iy-1].real *= tmpVar; 
                  //pSlc[ix-1][iy-1].imag *= tmpVar;
		  ii=ix-1; jj = iy-1;
		  get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);
                  ((float *)pSlc)[offsRe] *= tmpVar;
                  ((float *)pSlc)[offsIm] *= tmpVar;


                  //foutput3(iw,nx_taper-ix+1,iy           )=foutput3(iw,nx_taper-ix+1,iy           )*taperx(ix)*taperx(iy)
                  //pSlc[nx_taper-ix+1-1][iy-1].real *= tmpVar; 
                  //pSlc[nx_taper-ix+1-1][iy-1].imag *= tmpVar; 
                  ii=nx_taper-ix+1-1; jj = iy-1;
		  get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);
                  ((float *)pSlc)[offsRe] *= tmpVar;
                  ((float *)pSlc)[offsIm] *= tmpVar;

                  //foutput3(iw,ix,           ny_taper-iy+1)=foutput3(iw,ix,           ny_taper-iy+1)*taperx(ix)*taperx(iy)
                  //pSlc[ix-1][ny_taper-iy+1-1].real *= tmpVar; 
                  //pSlc[ix-1][ny_taper-iy+1-1].imag *= tmpVar; 
                  ii=ix-1; jj = ny_taper-iy+1-1;
		  get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);
                  ((float *)pSlc)[offsRe] *= tmpVar;
                  ((float *)pSlc)[offsIm] *= tmpVar;

                  //foutput3(iw,nx_taper-ix+1,ny_taper-iy+1)=foutput3(iw,nx_taper-ix+1,ny_taper-iy+1)*taperx(ix)*taperx(iy)
                  //pSlc[nx_taper-ix+1-1][ny_taper-iy+1-1].real *= tmpVar;
                  //pSlc[nx_taper-ix+1-1][ny_taper-iy+1-1].imag *= tmpVar;
                  ii=nx_taper-ix+1-1; jj = ny_taper-iy+1-1;
		  get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);
                  ((float *)pSlc)[offsRe] *= tmpVar;
                  ((float *)pSlc)[offsIm] *= tmpVar;

	       } //end do
             } //end do



             //foutput3(iw,nx_taper+1:nx_fft,          1:ny_fft)=cmplx(0.0,0.0)
             for(ii = nx_taper+1; ii <= pReGlb->nx_fft; ii++) {
	       for(jj  = 1; jj <= pReGlb->ny_fft; jj++) {
		   //pSlc[ii-1][jj-1].real = 0;
		   //pSlc[ii-1][jj-1].imag = 0;
		  get1DoffsReIm(szX, szY, ii-1, jj-1, &offsRe, &offsIm);
                  ((float *)pSlc)[offsRe] *= tmpVar;
                  ((float *)pSlc)[offsIm] *= tmpVar;

	       } // for jj
	     } // for(ii

             //foutput3(iw,1:nx_fft,          ny_taper+1:ny_fft)=cmplx(0.0,0.0)
             for(ii = 1; ii <= pReGlb->nx_fft; ii++) {
	       for(jj  = ny_taper+1; jj <= pReGlb->ny_fft; jj++) {
		   //pSlc[ii-1][jj-1].real = 0;
		   //pSlc[ii-1][jj-1].imag = 0;
		  get1DoffsReIm(szX, szY, ii-1, jj-1, &offsRe, &offsIm);
                  ((float *)pSlc)[offsRe] *= tmpVar;
                  ((float *)pSlc)[offsIm] *= tmpVar;
	       } // for jj
	     } // for(ii


             int offsOutp;
             //------- cutt off the origo extra part of the domain  ---
             //------- store the real part only in the real outp array 
	     for(ix = 1; ix <= pReGlb->nx_in; ix++) { //do ix=1,nx_in
                for(iy = 1; iy <= pReGlb->ny_in; iy++)  { // do iy=1,ny_in
			//migrated(ix,iy,izSlab)=migrated(ix,iy,izSlab)+&
			//&real(sum(foutput3(1:nwHlocal(rank+1),nx_origo+ix,ny_origo+iy)))
			
                        // migrChunk[ix-1][iy-1] += pSlc[pReGlb->nx_origo+ix-1][pReGlb->ny_origo+iy-1].real;
                        // dms, now replace "+=" by "=", the reduction is to be done by thread #3 on the right node
                        // also the summation level by level afterwards
		    //pOutpRe[ix-1][iy-1] = pSlc[pReGlb->nx_origo+ix-1][pReGlb->ny_origo+iy-1].real;
                    offsOutp = ix*pReGlb->ny_in + iy;

                  ii=pReGlb->nx_origo+ix-1  -1; jj = pReGlb->ny_origo+iy-1 -1;
		  get1DoffsReIm(szX, szY, ii, jj, &offsRe, &offsIm);
                  pOutpRe[offsOutp] = ((float *)pSlc)[offsRe];
 
 
		} 
	     } 



//=== to test: delete locally allocated arrays ---

    delete [] pSlc;
    delete [] pOutpRe; 

//==== to test:delete locally allocated arrays ---

       //------- delete the work-array for the FFT ---
       delete [] pWrk;
       delete [] u1;


       //--- free vMin[] and vMax[] -------
       delete [] vMax;
       delete [] vMin;

       //---- delete here the aux vectors, which are - otherwise - global --
       delete [] taperx;
       delete [] ky2;
       delete [] ky;
       delete [] kx2;
       delete [] kx;
       delete [] wH;

    delete pReGlb; // del glbVars struct

    return 1;
}
*************/

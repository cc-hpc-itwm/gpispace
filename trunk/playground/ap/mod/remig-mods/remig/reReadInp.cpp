/*********************************************

   reReadInp.cpp
           routine(s) to read & distribute the inp data


*********************************************/

#include <stdio.h>
#include <string.h>
#include <fhglog/fhglog.hpp>


#include <fvm-pc/pc.hpp>

//---------- include some definitions used here --------------

extern "C" {
  #include "/opt/cluster/Intel/ice/3.0.023/cmkl/9.0/include/mkl_types.h"
}
#include "fftw3.h" //sfftw_plan_dft_1d


#include "utls.h"
#include "reReadInp.h"


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



//------ readAndDistributeInputData ------
static int readAndDistributeInputData(cfg_t *pCfg, TReGlbStruct *pReG, int *nwHlocal, int *nwHdispls)
//  - read the time-data and do 1D FFT time -> freq
//  - distribute the freq inp array on all CPUs 
//           (later on - the right partt of the whole array only)
{
    const float pi=3.1415926;
    const int LEN = 100;

    //int i, j, k, it, 
    int it, iSz, jSz, kSz, ix, iy, iz, iw;

    FILE *pfData;   // ptr datafile[]

    int bytes; 

    int szX = pReG->nx_fft;
    int szY = pReG->ny_fft;
    int szZ = pReG->nwH;

       int rank = fvmGetRank();
       int size = fvmGetNodeCount();
        fftwf_plan plan_trace;             //!Input 1D FFT plan /

	float     **data;                           //!zero offset image
	MKL_Complex8  **foutput1D_in; //, **foutput1D_out; //!for 1D FFT

	//------ alloc shmem chunk, where to place the whole data cube ---
	fvmSize_t shmemLclSz = szX*szY*szZ *sizeof(MKL_Complex8);
        //fvmAllocHandle_t hLclShMem =fvmLocalAlloc(shmemLclSz);   // not necessary
        unsigned char *pShMdatCube = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr
        bzero(pShMdatCube, shmemLclSz);

	//--- alloc scratzch handle, for now of the same size -- 
        fvmAllocHandle_t hScra; 
        hScra = fvmLocalAlloc(shmemLclSz);

        //fvmOffset_t vmDestOffs;  //--- dest offs VM, see below, where the transfer occurs 
        //fvmShmemOffset_t shmemSrcOffs; // lcl shmem src offs (in the data cube)
        
        //fvmSize_t transfrSZbytes; // bytes to transfer 
        //fvmCommHandle_t commH;    // comm handle for glbPut   
        int offset; // offset in the 3D data array
        fvmCommHandleState_t commStatus;

    //------ first check the size of the full pFreqInpCube[][][] array :
    //------  does it really fit into the space separated for it 
    //if((unsigned long) szX*szY*szZ > szFreqInpCube) {
    //    pv4d_printf("\n [%d] freq inp cube too big \n", rank);
        // exit (0);
       //}

    //---- read & FFT  on CPU#0 only ----
//    if(rank == 0) {  // sdpa should run this only on node#0

        //--------- set some auxiliary arrays, fftw-plan, etc.. 
	jSz = pReG->nt; 
        iSz = pReG->nx_out; 
        if(utlAlloc2DfloatArray(&data, iSz, jSz) == 0) {  // C-indexing in data[][], 
                                                          // time (i.e. j-index) -fastest direction
	   printf("\n [%d] Unable to allocate zero offset image cube \n", rank);
	   return -1;
        } //end if
        utlSetToZero2DfloatArray(data, iSz, jSz); //data=0.0

        iSz = pReG->nt_fft + 1;
        MKL_Complex8 *trace = new MKL_Complex8 [iSz];

	//! Create taper
        //allocate(tapert(1:nwH))
        iSz = pReG->nwH+1;
        float *tapert = new float [iSz];
        tapert[0] = 0;

        for(iw=1; iw <= pReG->nwH; iw++) { //do iw=1,nwH
	   // if((iw.ge.(nwmin-nwmin)).and.(iw.le.(nwmin2-nwmin)))then
	    if((iw >= (pReG->nwmin-pReG->nwmin)) && (iw <= (pReG->nwmin2-pReG->nwmin))) { 
                tapert[iw]=0.53824-0.46164*cos(pi*(iw)/(pReG->nwmin2-pReG->nwmin));
            }
	    else{
		//if((iw.ge.nwmax2).and.(iw.le.nwmax))then
                if((iw >= pReG->nwmax2) && (iw <= pReG->nwmax)) {
		    tapert[iw]=0.53824-0.46164*cos(pi*(iw-pReG->nwmax2+1)/(pReG->nwmax-pReG->nwmax2)-pi); 
                
                } 
                else {
		    tapert[iw]=1.0;
                }
            }//endif
	} //end do

	//! Plan 1D FFTW
        plan_trace = fftwf_plan_dft_1d(pReG->nt_fft,(fftwf_complex *) &trace[1],(fftwf_complex *) &trace[1],-1,FFTW_MEASURE);

        //------------------- open datafile ------
	iz = pReG->nt*pReG->nx_out; // dms -> record size, to me - irrelevant  
        if((pfData=fopen(pCfg->data_file, "rb"))==NULL) {
	     printf("\n [%d] Can not open %s", rank, pCfg->data_file); 
	     return -1;
        }
		DLOG(DEBUG, "reading data from: " << pCfg->data_file);

        //------ another auxiliary array -----------
        jSz = pReG->nx_out; //+1; 
        iSz = pReG->nwH; //+1;
        utlAlloc2DcomplArray(&foutput1D_in, iSz, jSz);  //dms, permute here the indices and use C-indexing
        
        //----------- read the input file in y=const lines --
        for(iy =1; iy <= pReG->ny_out;iy++) { //do iy=1,ny_out

            fseek(pfData, (long int)  (iy-1)*(pReG->nx_out*pReG->nt)*sizeof(float), SEEK_SET); 
            bytes = fread((float *) &(data[0][0]), sizeof(float), pReG->nx_out*pReG->nt, pfData);

            //---------- 1D fft foe each trace at a point (ix, iy) in the z=0 plane
	    for(ix=1; ix <= pReG->nx_out; ix++) { //do ix=1,nx_out               
		//trace=cmplx(0.0,0.0)
                iSz = pReG->nt_fft + 1;
                bzero(trace, iSz*sizeof(MKL_Complex8));

                for(it=1; it <= pReG->nt; it++) { // do it=1,nt
                    trace[it].real = data[ix-1][it-1];  // data[][] indexed from ZERO as well
                    trace[it].imag = 0.0;           
                } //end do

                fftwf_execute(plan_trace);  // 1D FFT time -> freq

                for(iw=1; iw <= pReG->nwH; iw++) { //do iw=1,nwH
                    foutput1D_in[iw-1][ix-1].real = trace[pReG->nwmin+iw-1].real * tapert[iw]/pReG->nt_fft;
                    foutput1D_in[iw-1][ix-1].imag = trace[pReG->nwmin+iw-1].imag * tapert[iw]/pReG->nt_fft;

		    //foutput3[iw-1][ntaper+ix-1][ntaper+iy-1].real = foutput1D_out[iw-1][ix-1].real; 
		    //foutput3[iw-1][ntaper+ix-1][ntaper+iy-1].imag = foutput1D_out[iw-1][ix-1].imag; 

		     offset = //(unsigned long) 
		        (indxOffsetIn3DarrayInZplaneChunks(iw-1, pReG->ntaper+ix-1, pReG->ntaper+iy-1, szZ, szX, szY)*2);

		     //printf("\n [%d][%d][%d]", iw-1, ntaper+ix-1, ntaper+iy-1);
                     //((float*) pFreqInpCube)[offset] = foutput1D_in[iw-1][ix-1].real;                    
                     //((float*) pFreqInpCube)[offset+1] = foutput1D_in[iw-1][ix-1].imag;

                     // -- dms, no, put it in the shmem
                     ((float*) pShMdatCube)[offset] = foutput1D_in[iw-1][ix-1].real;                    
                     ((float*) pShMdatCube)[offset+1] = foutput1D_in[iw-1][ix-1].imag;
                  
               } //enddo
	    } //end do for(ix=1;

	} //for(iy =1; iy <= ny_out;iy++)  

        //--------- free aux arrays and close file --------
        jSz = pReG->nx_out; 
        iSz = pReG->nwH; 
        utlFree2DcomplArray(foutput1D_in, iSz, jSz);  //dms, permute here the indices and use C-indexing

        delete [] trace;
        delete [] tapert;

	jSz = pReG->nt; 
        iSz = pReG->nx_out; 
        utlFree2DfloatArray(data, iSz, jSz);


	fclose(pfData);

        fftwf_destroy_plan(plan_trace);

//    } //if(rank == 0)



  //---- now the whole data cube is in the shMemChunk, addressed by pShMdatCube --
    //---- distribute the result on all other nodes ---

        fvmOffset_t vmDestOffs;  //--- dest offs VM, see below, where the transfer occurs 
        fvmShmemOffset_t shmemSrcOffs; // lcl shmem src offs (in the data cube)
        
        fvmSize_t transfrSZbytes; // bytes to transfer 
        fvmCommHandle_t commH;    // comm handle for glbPut   
        fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;

 
    fvmSize_t shmemGlbSz = pCfg->nodalSharedSpaceSize;
    szX = pReG->nx_fft; szY = pReG->ny_fft;


    int iNd;

	DLOG(DEBUG, "distributing input data to the nodes...");
 
	for(iNd=0; iNd < size; iNd++) {  

	   vmDestOffs = iNd*shmemGlbSz + pCfg->ofsInp; // the offs on the remote node
           shmemSrcOffs = (nwHdispls[iNd+1]-1)*szX*szY*sizeof(MKL_Complex8);
           // szZ = nwHlocal[i+1]; // the number of frequencies, distributed on node #i
           // transfrSZbytes = szZ*szX*szY*sizeof(MKL_Complex8);
           transfrSZbytes = nwHlocal[iNd+1]*szX*szY*sizeof(MKL_Complex8);
           
           commH =  fvmPutGlobalData(hGlbVMspace,   //const fvmAllocHandle_t handle,
				     vmDestOffs, //const fvmOffset_t fvmOffset,
				     transfrSZbytes, //const fvmSize_t size,
				     shmemSrcOffs, //const fvmShmemOffset_t shmemOffset,
				     hScra); //const fvmAllocHandle_t scratchHandle);
	   
           commStatus  = waitComm(commH);
           if(commStatus != COMM_HANDLE_OK) return (-1);
       } 

	LOG(INFO, "input data has been read and distributed (executing node: " << rank << ")");


//        //-------- report that inp data has been read & distributed  ---
//
//        FILE *fp; 
//        char fn[100];
//        sprintf(fn, "/scratch/dimiter/sdpa/fTrace%d.txt", rank);
//        if((fp=fopen(fn, "at")) != NULL) {
//            //--- 
//            fprintf(fp, "\n ------------------------------------------------------------------------");
//            fprintf(fp, "\n        input data read on #%d and distributed, the tests were OK      \n", rank);
// 
//            fclose(fp);
//	}
	//--------- here you can do some checks, the check is OK !! ------
/*
        //FILE *fp; 
        //char fn[100];
        int iwLclStart, iwLclEnd;
        double valRe, valIm;

        sprintf(fn, "/scratch/dimiter/sdpa/fTrace%d.txt", rank);

        if((fp=fopen(fn, "at")) != NULL) {
            //--- 
            fprintf(fp, "\n ------ printing input data, checking the distribution ----\n");
            //------ set to zero our shMem chunk --
            szX = pReG->nx_fft;
            szY = pReG->ny_fft;
            szZ = pReG->nwH;
            shmemLclSz = szX*szY*szZ *sizeof(MKL_Complex8);
            bzero(pShMdatCube, shmemLclSz);

            //-- in the transfer use the same vars, but they will have the opposite meaning --
            //--   -> vmDestOffs will be now the src offs --
            //--   -> shMemSrcOffs is the dest offs, always == 0  (we transfer from VM-glb space to shMem)
            shmemSrcOffs = 0;
           
            for(iNd=0; iNd < size; iNd++) {
              vmDestOffs = iNd*shmemGlbSz + pCfg->ofsInp;
              transfrSZbytes = nwHlocal[iNd+1]*szX*szY*sizeof(MKL_Complex8);
             
              commH = fvmGetGlobalData(hGlbVMspace,    //const fvmAllocHandle_t handle,
				       vmDestOffs,     //const fvmOffset_t fvmOffset,
				       transfrSZbytes, //const fvmSize_t size,
				       shmemSrcOffs, //const fvmShmemOffset_t shmemOffset,
				       hScra); //const fvmAllocHandle_t scratchHandle);



             commStatus  = waitComm(commH);
             if(commStatus != COMM_HANDLE_OK) return (-1);

              iwLclStart =nwHdispls[iNd+1]-1;
              iwLclEnd =nwHdispls[iNd+1 +1]-1;
	      szZ = nwHlocal[iNd+1];
              for(iw = 0; iw < nwHlocal[iNd+1]; iw++) {
                 fprintf(fp, "\n --- ttlFrqs:%d inp data distributed to nd #%d, numFreqs here:%d --> iw_lcl:%d,  iw_glb:%d ----\n", 
                    pReG->nwH, iNd, nwHlocal[iNd+1], iw, iwLclStart+iw);  
                 for(ix = 0; ix < pReG->nx_fft;ix++) { 
                    for(iy = 0; iy < pReG->ny_fft;iy++) { //do iy=1,ny_out
                       offset = indxOffsetIn3DarrayInZplaneChunks(iw, ix, iy, szZ, szX, szY)*2;
                       valRe = ((float*) pShMdatCube)[offset];
                       valIm = ((float*) pShMdatCube)[offset+1];

                       fprintf(fp, "\n on [%d], nFrqs_here:%d, iw_lcl:%d, iw_glb:%d dat[ix:%d][iy:%d] -> re:%e im:%e", 
			       iNd, nwHlocal[iNd+1], iw, iwLclStart+iw, ix, iy, valRe, valIm);
                    } //for(iy =0; iy < pReG->ny_out;iy++)
                    fprintf(fp, "\n");
                 } //for(ix =0; ix < pReG->nx_out;ix++)
              } //for(iw = 0; iw < nwHlocal[iNd+1]; iw++)
	    } //for(iNd=0; iNd < size; iNd++) {          


	    fclose(fp);
        } // if((fp=fopen(fn, "wt")) != NULL) 
*/
        //-------- end checks, the check is OK -----------

     fvmLocalFree(hScra);
     //fvmLocalFree(hLclShMem); // free the local sh mem


    return 1;
}

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

/*
//--------- reApplInit --------------
int reApplInit(cfg_t *pCfg)
{

       //---- the struct with re-glb vars, replicate it on each node
       TReGlbStruct *pReGlb = new TReGlbStruct;


       //------- init re- glb vars and replicate them over the nodes ---
       initReGlbVars(pCfg, pReGlb);


       delete pReGlb; // delete re glb struct

    return 1;
}
*/

//----------- readAndDistributeInput ----------
int readAndDistributeInput(cfg_t *pCfg)
{
   //---- the struct with re-glb vars, replicate it on each node
       TReGlbStruct *pReGlb = new TReGlbStruct;
      // copy here re-glb vars from the VM-glb space, offs == ofsGlbDat == 0; 
       cpReGlbVarsFromVM(pCfg, pReGlb);


       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

       //------- set freq distributing structs --------
       int cSize = size+1; // fortran relics, seee the original
       int *pIWonND = new int[pReGlb->nwH];
       int *nwHlocal = new int [cSize];
       int *nwHdispls = new int [cSize+1];
       //int wHlcl_startIndx, wHlcl_endIndx;
       
       calcLoadDistribution(pReGlb, nwHlocal, nwHdispls);
       allocAndSetLclFrqDistributionStructs(pReGlb, nwHlocal, nwHdispls, pIWonND);
    
       //------ readAnd DistributeInpData()
       readAndDistributeInputData(pCfg, pReGlb, nwHlocal, nwHdispls );


       //---- delete freq distr structs ---
       delete [] pIWonND;
       delete [] nwHlocal;
       delete [] nwHdispls;

       delete pReGlb; // delete re glb struct

    return 1;
}




/*
//----------- readAndDistributeInpData ------------
int readAndDistributeInput(cfg_t *pCfg)
{

       //---- the struct with re-glb vars, replicate it on each node
       TReGlbStruct *pReGlb = new TReGlbStruct;


       //------- init re- glb vars and replicate them over the nodes ---
       initReGlbVars(pCfg, pReGlb);


       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

       //------- set freq distributing structs --------
       int cSize = size+1; // fortran relics, seee the original
       int *pIWonND = new int[pReGlb->nwH];
       int *nwHlocal = new int [cSize];
       int *nwHdispls = new int [cSize+1];
       //int wHlcl_startIndx, wHlcl_endIndx;
       
       calcLoadDistribution(pReGlb, nwHlocal, nwHdispls);
       allocAndSetLclFrqDistributionStructs(pReGlb, nwHlocal, nwHdispls, pIWonND);
    
       //------ readAnd DistributeInpData()
       readAndDistributeInputData(pCfg, pReGlb, nwHlocal, nwHdispls );


       //---- similar as for the freqs, set now depth levl distr structs ---
       //int *izOffs = new int [size];
       //int *pIZonND = new int [pReGlb->nz];

       //allocAnsSetDepthLvlsDistributionStructs(pReGlb, izOffs, pIZonND);

       //----- readAndDistributeVelocityModel()
       //readAndDistributeVelocityModel(); // no, do another object file 

       //------- delete depth levl distr structs --- 
       //delete [] izOffs;
       //delete [] pIZonND;

       //---- delete freq distr structs ---
       delete [] pIWonND;
       delete [] nwHlocal;
       delete [] nwHdispls;

     
       delete pReGlb; // delete re glb struct

    return 1;
}

*/

/*********************************************

   reApplInit.cpp
           routine(s) to initialize  re-


*********************************************/

#include <stdio.h>
#include <string.h>


#include <sdpa/modules/util.hpp>
#include <fvm-pc/pc.hpp>

//---------- include some definitions used here --------------

extern "C" {
  #include "/opt/cluster/Intel/ice/3.0.023/cmkl/9.0/include/mkl_types.h"
}

#include "pfafft.h"

#include "reApplInit.h"


//--------- readParamFile -------------
static int readParamFile(cfg_t *pCfg, TReGlbStruct *pReG)
{
    const int LEN = 100;
	int i;
	FILE *fp(NULL);
	char charatemp[LEN]; 
    char title[LEN];

        if((fp=fopen(pCfg->config_file, "r"))==NULL) {
		printf("\n Can not open file %s !!!\n", pCfg->config_file);
		return -1;
        }

            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");

	   //----- title ------------
	   bzero(title, LEN);   // dms -> my addition, ZERO title
           fscanf(fp, "%[^\n]", title); // printf("\n title:%s\n", title);
	  snprintf(pCfg->title, cfg_t::max_path_len, "%s", title);

	   fscanf(fp, "%\n");    

            //---- read 2 lines cooments ---
            for(i=0; i < 2; i++) { 
		fscanf(fp, "%[^\n]", charatemp); 
		fscanf(fp, "%\n");                 
            }

            //--------- dimension --------
	    fscanf(fp, "%d %d %d %d\n", 
                &pReG->nz, &pReG->nx_in, &pReG->ny_in, &pReG->nt);
	    fscanf(fp, "%\n");    

            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");

            //--------- smapling increments --------
	    fscanf(fp, "%f %f %f %f\n", 
                   &pReG->dz, &pReG->dx, &pReG->dy, &pReG->dt);
	    fscanf(fp, "%\n");    
            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");

            //--------- aperture --------
	    fscanf(fp, "%d %d\n", &pReG->nx_ap, &pReG->ny_ap);
	    fscanf(fp, "%\n");    

            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");

char datafile_tmp[LEN];
 int sufile;

	     //----- zero offset image ------------
	     bzero(datafile_tmp, LEN);   // dms -> my addition, ZERO the filename
             fscanf(fp, "%s %d\n", datafile_tmp, &sufile);
	     fscanf(fp, "%\n");    

         snprintf(pCfg->data_file, cfg_t::max_path_len, "%s/%s", pCfg->prefix_path, datafile_tmp);

            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");

char velfile_tmp[LEN];
int feet, vbackgr;

	     //----- velocity file ------------
	     bzero(velfile_tmp, LEN);   // dms -> my addition, ZERO the filename
             fscanf(fp, "%s %d %d\n", velfile_tmp, &feet, &vbackgr);  
	     fscanf(fp, "%\n");    

         snprintf(pCfg->velocity_file, cfg_t::max_path_len, "%s/%s", pCfg->prefix_path, velfile_tmp);

            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");

	     //----- Output, demigrated section (dms -> Remigrated, not De- ??) ---

char outfile[LEN];

	     bzero(outfile, LEN);   // dms -> my addition, ZERO the filename
             fscanf(fp, "%s\n", outfile);  
	     fscanf(fp, "%\n");    

         snprintf(pCfg->output_file, cfg_t::max_path_len, "%s/%s", pCfg->prefix_path, outfile);

            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");


int propagator;
            //--------- Propagator type --------
	    fscanf(fp, "%d\n", &propagator);
	    fscanf(fp, "%\n");    

            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");

            //--------- Hamming filter frequencies --------
	    fscanf(fp, "%f %f %f %f\n", 
                  &pReG->fmin, &pReG->fmin2, &pReG->fmax2, &pReG->fmax);
	    fscanf(fp, "%\n");    

            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");

	    //--------- Size of wavenumber taper -------
	    fscanf(fp, "%d\n", &pReG->ntaper);
	    fscanf(fp, "%\n");    

            //------ a line comment -----------
	      fscanf(fp, "%[^\n]", charatemp);
	      fscanf(fp, "%\n");

int verbose;
	    //--------- Verbose level -------
	    fscanf(fp, "%d\n", &verbose);
	    fscanf(fp, "%\n");    

	fclose(fp);


    return 1;
}

//------- writeCheckParamFile -----------
static int writeCheckParamFile(cfg_t *pCfg, TReGlbStruct *pReG)
// dms -> my addition, just to debug
{
    const int LEN = cfg_t::max_path_len;
	int i;
	FILE *fp;
        char fn[LEN];

	//===== dms, write what you have read -> on each CPU ==
        snprintf(fn, LEN, "%s/%s.%d", pCfg->prefix_path, "checkParFile", fvmGetRank());

                if((fp=fopen(fn, "w"))==NULL) {
		    printf("\n Can not open file %s !!!\n", fn);
				return -1;
                }

	        fprintf(fp, "*******************************************\n");
                fprintf(fp, "%s, read on CPU #0  \n", pCfg->title);
	        fprintf(fp, "*******************************************\n");

		fprintf(fp, "# Model size: NZ,NX,NY,NT [#]\n");
                fprintf(fp, "%d %d %d %d\n", pReG->nz, pReG->nx_in, pReG->ny_in, pReG->nt);

                fprintf(fp, "# Increments: DZ,DX,DY,DT [m], [m], [m], [s]\n");
                fprintf(fp, "%f %f %f %f\n", pReG->dz, pReG->dx, pReG->dy, pReG->dt);

                fprintf(fp, "# Aperture [#]\n");
                fprintf(fp, "%d %d\n", pReG->nx_ap, pReG->ny_ap);


 int sufile = 1;
		fprintf(fp, "# Image filename, sufile?\n");
		fprintf(fp, "%s %d\n", pCfg->data_file, sufile);

 int feet=0, vbackgr=0;
                fprintf(fp, "# Velocity filename, Feet(0=no,1=yes)? Background(vmin,vrms=0,1)?\n");
                fprintf(fp, "%s %d %d\n", pCfg->velocity_file, feet, vbackgr);

                fprintf(fp, "# Remigrated output filename\n");
                fprintf(fp, "%s\n", pCfg->output_file);

 int propagator = 1;
                 fprintf(fp, "# Propagator, 1=PS, 2=SSF, 3=GSP1, 4=GSP2\n");
                fprintf(fp, "%d\n", propagator);

                fprintf(fp, "# Temporal Hamming frequencies: f1,f2,f3,f4 [Hz]\n");
                fprintf(fp, "%f %f %f %f\n", pReG->fmin,pReG->fmin2,pReG->fmax2,pReG->fmax);

                fprintf(fp, "# Spatial taper size\n");
                fprintf(fp, "%d\n", pReG->ntaper);

 int verbose = 0;
                fprintf(fp, "#verbose level:0,1,2\n");
                fprintf(fp, "%d\n", verbose);

	        fclose(fp);

    return 1;
}

//-------- defineAndCalcParameters ------------
static int defineAndCalcParameters(TReGlbStruct *pReG)
{
    const float pi=3.1415926;

    int tmp1, tmp2;

    pReG->iupd=1; //!Propagation convension (depends on corresponding FFT convension) // dms, local
    pReG->nx_out=pReG->nx_in+2*pReG->nx_ap;
    pReG->ny_out=pReG->ny_in+2*pReG->ny_ap;

    //fftolen_(nx_out+2*ntaper,2*(nx_out+2*ntaper), &nx_fft);
    tmp1 = pReG->nx_out+2*pReG->ntaper; tmp2 = 2*(pReG->nx_out+2*pReG->ntaper);
    fftolen_(&tmp1,&tmp2, &pReG->nx_fft);

    //fftolen_(ny_out+2*ntaper,2*(ny_out+2*ntaper), &ny_fft);
    tmp1=pReG->ny_out+2*pReG->ntaper; tmp2=2*(pReG->ny_out+2*pReG->ntaper);
    fftolen_(&tmp1, &tmp2, &pReG->ny_fft);

    pReG->nx_origo=pReG->ntaper+pReG->nx_ap;
    pReG->ny_origo=pReG->ntaper+pReG->ny_ap;
    pReG->dkx=2.0*pi/( ((float)(pReG->nx_fft-0)) * pReG->dx );
    pReG->dky=2.0*pi/( ((float)(pReG->ny_fft-0)) * pReG->dy );
    pReG->max1d=MAX(pReG->nx_fft,pReG->ny_fft);   // MAX(.,.) defined in cwp.h
  

    pReG->nt_fft=2*pReG->nt;             //! Padding, overkill if nwmax << nw
    pReG->df = 1.0/(pReG->nt_fft*pReG->dt);    //! Freq. sampling
    pReG->dw = 2.0*pi*pReG->df;          //! Circular freq. sampling
    pReG->nw=pReG->nt_fft/2+1;           //! Number of freq.samples

    //! Initial cric. freq
    pReG->nwmin = (int)(pReG->fmin/pReG->df)+1;
    pReG->nwmin2 = (int)(pReG->fmin2/pReG->df)+1;
    pReG->nwmax2 = (int)(pReG->fmax2/pReG->df);
    pReG-> nwmax = (int)(pReG->fmax/pReG->df);
  
    //! Circ. freqs corrected
    pReG->nwmin  = MAX(2,     pReG->nwmin );
    pReG->nwmin2 = MAX(pReG->nwmin, pReG->nwmin2);
    pReG->nwmax  = MIN(pReG->nw,    pReG->nwmax );
    pReG->nwmax2 = MIN(pReG->nwmax, pReG->nwmax2);
  
    //! Number of circ. freqs to process
    pReG->nwH = pReG->nwmax-pReG->nwmin+1;


    return 1;
}


//----------- printParameters ----------
static int printParameters(cfg_t *pCfg, TReGlbStruct *pReG)
{
    //!*******************************
    //! Print some parameters to stout
    //!*******************************
    const int LEN = cfg_t::max_path_len;
	int i;
	FILE *fp;
    char fn[LEN];

        snprintf(fn, LEN, "%s/%s.%d", pCfg->prefix_path, "checkParFile-print", fvmGetRank());

	//===== dms, write what you have read -> on each CPU ==

                if((fp=fopen(fn, "a"))==NULL) {
		    printf("\n Can not open file %s !!!\n", fn);
		    //exit (1);
                }

		//printf(fp, "\nNumber of CPUs..............%d",size);
		//printf("\n Parameterfile:             %s", parfile);

		fprintf(fp, "\n \n\n\n printing parameters\n");

            fprintf(fp, "\n   %s  ", pCfg->title);
            fprintf(fp, "\n Zero offset demig file:\t %s \t (%5d, %5d, %5d)",
		                                pCfg->data_file,pReG->nt,pReG->nx_out,pReG->ny_out);
            fprintf(fp, "\n Velocity file (x,y,z):\t %s \t (%5d, %5d, %5d)",
		                                pCfg->velocity_file,pReG->nx_in,pReG->ny_in,pReG->nz);
            fprintf(fp, "\n Output file (z,x,y):\t %s \t (%5d, %5d, %5d)",
	                                    pCfg->output_file, pReG->nz,pReG->nx_in,pReG->ny_in);
	    // if (propagator == PS) {
	        fprintf(fp, "\n Propagator: PS");
		//}

/*
            if (propagator == SSF){
                printf("\n Propagator: SSF");
	    }
            if (propagator == GSP1){
                printf("\n Propagator: GSP1");
            }
            if (propagator == GSP2){
                printf("\n Propagator: GSP2");
            }
            if (sufile == 1) {
                printf("\nDatafile is on su-format...");
	    }
            printf("\nVerbose level...............%d",verbose);
            printf("\nVerbose node................%d",verboseNode);
*/
            fprintf(fp, "\n\n*******************************************");
            fprintf(fp, "\nImage parameters");
            fprintf(fp, "\nnz..........................%d",pReG->nz);
            fprintf(fp, "\nnx in.......................%d",pReG->nx_in);
            fprintf(fp, "\nny in.......................%d",pReG->ny_in);
            fprintf(fp, "\nnx ap.......................%d",pReG->nx_ap);
            fprintf(fp, "\nny ap.......................%d",pReG->ny_ap);
            fprintf(fp, "\nnx out......................%d",pReG->nx_out);
            fprintf(fp, "\nny out......................%d",pReG->ny_out);
            fprintf(fp, "\nnt..........................%d",pReG->nt);
            fprintf(fp, "\ndz..........................%16.3f",pReG->dz);
            fprintf(fp, "\ndx..........................%16.3f",pReG->dx);
            fprintf(fp, "\ndy..........................%16.3f",pReG->dy);
            fprintf(fp, "\ndt..........................%16.3f",pReG->dt);
            fprintf(fp, "\n\nFFT parameters");
            fprintf(fp, "\ndf..........................%16.3f",pReG->df);
            fprintf(fp, "\ndw..........................%16.3f",pReG->dw);
            fprintf(fp, "\nnw..........................%d",pReG->nw);
            fprintf(fp, "\ndkx..........................%19.3e",pReG->dkx);
            fprintf(fp, "\ndky..........................%19.3e",pReG->dky);
            fprintf(fp, "\nnt_fft......................%d",pReG->nt_fft);
            fprintf(fp, "\nnx_fft......................%d",pReG->nx_fft);
            fprintf(fp, "\nny_fft......................%d",pReG->ny_fft);
	    fprintf(fp, "\n\nFilters");
            fprintf(fp, "\nHamming window frequency....   [%5.1f, %5.1f, %5.1f, %5.1f ] ",
		                                              pReG->fmin,pReG->fmin2, pReG->fmax2,pReG->fmax);
            fprintf(fp, "\nHamming window indexes......   [%5d, %5d, %5d, %5d ] ",
		   pReG->nwmin,pReG->nwmin2,pReG->nwmax2,pReG->nwmax);
            fprintf(fp, "\nnwH.........................%d",pReG->nwH);

	    fclose(fp);


    return 1;
}


//--------- initReGlbVars ---------------
static int initReGlbVars(cfg_t *pCfg, TReGlbStruct *pReGlb)
{
	  int retval(1);

      retval = readParamFile(pCfg, pReGlb);
	  if (retval != 1) return -1;

	  retval = writeCheckParamFile(pCfg, pReGlb);
      if (retval != 1) return -1;
 
      retval = defineAndCalcParameters(pReGlb);
      if (retval != 1) return -1;

      retval = printParameters(pCfg, pReGlb);
      if (retval != 1) return -1;
    
       
       //------ now the structure is filled-in, replicate it everywhere !!
       int rank = fvmGetRank();
       int size = fvmGetNodeCount();

       fvmSize_t transfrSZbytes = sizeof(TReGlbStruct);

       fvmCommHandle_t commH;
       fvmAllocHandle_t hGlbVMspace = pCfg->hndGlbVMspace;
       fvmSize_t shmemSz = pCfg->nodalSharedSpaceSize;


	   fvm::util::local_allocation hScra(transfrSZbytes);

       fvmOffset_t vmDestOffs;
       fvmShmemOffset_t shmemSrcOffs=0;
       

       unsigned char *pShMem = (unsigned char *) fvmGetShmemPtr(); // sh mem ptr



       memcpy(pShMem, pReGlb, transfrSZbytes); // copy my glb struct to sh mem, offs=shmemSrcOffs=0

       int iNd;
       for(iNd = 0; iNd < size; iNd++) {
           
           vmDestOffs = iNd * shmemSz + pCfg->ofsGlbDat; //+ofsGlbDat==0;


           commH =  fvmPutGlobalData(hGlbVMspace,   //const fvmAllocHandle_t handle,
				     vmDestOffs, //const fvmOffset_t fvmOffset,
				     transfrSZbytes, //const fvmSize_t size,
				     shmemSrcOffs, //const fvmShmemOffset_t shmemOffset,
				     hScra); //const fvmAllocHandle_t scratchHandle);

           if (waitComm(commH) != COMM_HANDLE_OK)
			 return (-1);
       } //for(iNd = 0; iNd < size; iNd++)

    return 1;
}


//--------- reApplInit --------------
int reApplInit(cfg_t *pCfg)
{

       //---- the struct with re-glb vars, replicate it on each node
       TReGlbStruct *pReGlb = new TReGlbStruct;


       //------- init re- glb vars and replicate them over the nodes ---
       int retval = initReGlbVars(pCfg, pReGlb);


       delete pReGlb; // delete re glb struct

    return retval;
}

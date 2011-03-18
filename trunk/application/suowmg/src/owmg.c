#include "owmg.h"

unsigned long long t1, t2, tModInit=0, tModRun=0;


owmgData *owmg_init(char *medium, char *propagator, int nx, int ny, int nz, float dx, float dy, float dz, 
		    int iw1, int iw4, int nwH, float dw, float latSamplesPerWave, float vertSamplesPerWave)
{

  owmgData *data = NULL;

  data = (owmgData *)malloc(sizeof(owmgData));

  if (!strcmp(medium,"ISO")) {
    data->medium=ISO;
    fprintf(stderr, "Medium is ISO\n");
  } else if (!strcmp(medium,"VTI")) {
    data->medium=VTI;
    fprintf(stderr, "Medium is VTI\n");
  } else if (!strcmp(medium,"TTI")) {
    data->medium=TTI;
    fprintf(stderr, "Medium is TTI\n");
  } else {
    data->medium=-1;
    fprintf(stderr, "Medium is unknown\n");
  }

  if (!strcmp(propagator,"PS")) {
    data->propagator=PS;
    fprintf(stderr, "Propagator is PS\n");
  } else if (!strcmp(propagator,"SSF")) {
    data->propagator=SSF;
    fprintf(stderr, "Propagator is SSF\n");
  } else if (!strcmp(propagator,"FFD2")) {
    data->propagator=FFD2;
    fprintf(stderr, "Propagator is FFD2\n");
  } else if (!strcmp(propagator,"FFD3")) {
    data->propagator=FFD3;
    fprintf(stderr, "Propagator is FFD3\n");
  } else {
    data->propagator=-1;
    fprintf(stderr, "Propagator is unknown\n");
  }

  // Assign constants
  data->nx   = nx;
  data->ny   = ny;
  data->nz   = nz;
  data->dx   = dx;
  data->dy   = dy;
  data->dz   = dz;
  data->iw1  = iw1;
  data->iw4  = iw4;
  data->nwH  = nwH;
  data->dw   = dw;
  data->latSamplesPerWave  =  latSamplesPerWave;
  data->vertSamplesPerWave = vertSamplesPerWave;

  data->nxf   = npfao(data->nx,2*data->nx);
  data->nyf   = npfao(data->ny,2*data->ny);

  data->avgList = (averageStruct **)malloc(data->nxf*data->nyf*sizeof(averageStruct *));
  for ( int i=0; i<data->nxf*data->nyf; i++) {
    data->avgList[i] = NULL;
  }
  

  /* Allocate memory */
  fprintf(stderr, "Allocating memory\n");
  data->imageCubeUD = allocfloat3(data->nz,data->ny,data->nx);
  data->imageCubeDD = allocfloat3(data->nz,data->ny,data->nx);
  data->src         = allocfloatcomplex3(data->nwH,data->nyf,data->nxf);
  data->rec         = allocfloatcomplex3(data->nwH,data->nyf,data->nxf);

  data->vPCube  = allocfloat3(data->nz,data->nyf,data->nxf);
  data->eCube   = allocfloat3(data->nz,data->nyf,data->nxf);
  data->dCube   = allocfloat3(data->nz,data->nyf,data->nxf);

  if (data->medium==ISO) {
    if (data->propagator==FFD3) {
      data->mem = alloc_iso_ffd3(data->nxf, data->nyf);
    }
  } else if (data->medium==VTI) {
    if (data->propagator==FFD3) {
      
    }
  } else if (data->medium==TTI) {
    if (data->propagator==FFD3) {
    }
  }


  return data;
 
}


void owmg_propagate(owmgData *data)
{
  int ix, iy, iz, iw, iwH, izMin;
  int propagatorA, nzA, nxA, nyA, nxf, nyf, nxfA, nyfA;
  int nxA_prev, nyA_prev;
  float w, f, dzA, dxA, dyA, vMin, vMax, vP, vB, k0;
  float *kx2A, *ky2A, *vMin_iz;
  float **vP2D, **taperXY, **taperXYA;
  float *srcPrev, *recPrev;
  fftwf_complex *src2D, *rec2D;
  fftwf_plan r1=NULL, s1=NULL, r2=NULL, s2=NULL;
  unsigned long long res, tStart, tEnd, t1, t2, t11, t12;
  unsigned long long tInit, tPlan, tInterpDown, tModel, tCoeff, tProp, tTaper, tCopy, tInterpUp, tImage1, tImage2;
  unsigned long long tPS, tSSF, tFFD3;
  timeFFD3 *time;
  tInit = tPlan = tInterpDown = tModel = tCoeff = tProp = tTaper = tCopy = tInterpUp = tImage1 = tImage2 = 0;
  tPS = tSSF = tFFD3 = 0;

  if (data->propagator == FFD3) {
    time = (timeFFD3 *)malloc(sizeof(timeFFD3));
    time->tConj1   = 0;
    time->tXM1     = 0;
    time->tXBound1 = 0;
    time->tXTri1   = 0;
    time->tXM2     = 0;
    time->tXBound2 = 0;
    time->tXTri2   = 0;
    time->tYM1     = 0;
    time->tYBound1 = 0;
    time->tYTri1   = 0;
    time->tYM2     = 0;
    time->tYBound2 = 0;
    time->tYTri2   = 0;
    time->tConj2   = 0;
  }
  get_rtc_res_(&res);
  get_rtc_(&t1);
  get_rtc_(&tStart);

  // Copy of data->nxf/nyf, they are used so frequent
  nxf=data->nxf;
  nyf=data->nyf;

  // Allocate FFTW work storage
  src2D   = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * nxf * nyf);
  rec2D   = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * nxf * nyf);

  // Allocate srcPrev and recPrev
  srcPrev = allocfloat1(2*nyf*nxf);
  recPrev = allocfloat1(2*nyf*nxf);

  // Allocate model storage
  vP2D    = allocfloat2(nyf, nxf);

  // Allocate spatial taper
  {
    taperXY  = allocfloat2(nyf, nxf);
    taperXYA = allocfloat2(nyf, nxf);

    float alpha = 0.1;
    int labsorb = 20;
    getTaperXY(taperXY, nxf, nyf, data->nx, data->ny, alpha, labsorb);
  }

  // Allocate vectors of wavenumbers (nxf/nyf is the largest possible value)
  kx2A=allocfloat1(nxf);
  ky2A=allocfloat1(nyf);

  // Allocate vMin vector and populate it
  vMin_iz = allocfloat1(data->nz);
  for ( iz=0; iz<data->nz; iz++ ) {
    vMin_iz[iz] = minval(data->vPCube[iz][0], nxf*nyf);;
  }

  // Zero image cubes
  for ( ix=0; ix<data->nx*data->ny*data->nz; ix++) {
    data->imageCubeUD[0][0][ix]=0.0f;
  }
  for ( ix=0; ix<data->nx*data->ny*data->nz; ix++) {
    data->imageCubeDD[0][0][ix]=0.0f;
  }

  // Convert from velocity to slowness
  for ( ix=0; ix<data->nxf*data->nyf*data->nz; ix++) {
    data->vPCube[0][0][ix] = 1.0f/data->vPCube[0][0][ix];
  }

  get_rtc_(&t2); tInit += t2-t1;
    


  izMin=0;

  // Frequency loop
  for ( iw=data->iw1; iw<=data->iw4; iw++) {

    // Reset for new frequency
    w=iw*data->dw;
    f=w/(2.0f*(float)PI);
    fprintf(stderr, "f=%f\n", f);
    iwH=iw-data->iw1;
    iz=izMin;
    nxA_prev=0;
    nyA_prev=0;
    

    // Depth loop
    while ( iz<data->nz ) {

      if ( (iz>20) && TRUE) {
	nzA=1;
	vMin=vMin_iz[iz];
	while (TRUE) {
	  if (nzA == data->nz-iz) {
	    // Maximum depth reached
	    break;
	  }
	  if (fminf(vMin, vMin_iz[iz+nzA])/(data->vertSamplesPerWave*f) < (nzA+1)*data->dz) {
	    // Sampling criteria violated
	    break;
	  }
	  // Safe to update vMin and nzA
	  vMin=fminf(vMin, vMin_iz[iz+nzA]);
	  nzA++;
	}
	dzA = data->dz*nzA;
	dxA = dyA = vMin/(data->latSamplesPerWave*f);
	
	// Adujst nxA and nyA
	nxA = ceilf(((data->nx-1)*data->dx)/dxA)+1;
	nyA = ceilf(((data->ny-1)*data->dy)/dyA)+1;
	nxfA = npfao(nxA,2*nxA);
	nyfA = npfao(nyA,2*nyA);
	nxA  = nxfA;
	nyA  = nyfA;
	dxA  = ((data->nx-1)*data->dx)/(nxA-1);
	dyA  = ((data->ny-1)*data->dy)/(nyA-1);
	if ( dxA<data->dx || nxA > data->nx) {
	  dxA = data->dx;
	  nxA = data->nx;
	  nxA = data->nx;
	  nxfA = nxf;
	}
	if ( dyA<data->dy || nyA > data->ny) {
	  dyA = data->dy;
	  nyA = data->ny;
	  nyA = data->ny;
	  nyfA = nyf;
	}
      } else {
	dxA = data->dx;
	nxA = data->nx;
	nxfA = nxf;
	dyA = data->dy;
	nyA = data->ny;
	nyfA = nyf;
	nzA=1;
	dzA=data->dz;
      }


      if (nxfA>nxf) fprintf(stderr, "ERROR: nxfA larger than nxf, nxfA=%i, nxf=%i\n", nxfA, nxf);
      if (nyfA>nyf) fprintf(stderr, "ERROR: nyfA larger than nyf, nyfA=%i, nyf=%i\n", nyfA, nyf);

      if ( (nxA != nxA_prev) || (nyA != nyA_prev) ) {
         // Create new FFTW plans - crucial to do this before copy/interpolation
         get_rtc_(&t1);
         if (s1!=NULL) {
            fftwf_destroy_plan(s1);
            fftwf_destroy_plan(r1);
            fftwf_destroy_plan(s2);
            fftwf_destroy_plan(r2);
         }
         s1 = fftwf_plan_dft_2d(nyfA,nxfA,src2D,src2D,-1,FFTW_MEASURE);
         r1 = fftwf_plan_dft_2d(nyfA,nxfA,rec2D,rec2D,-1,FFTW_MEASURE);
         s2 = fftwf_plan_dft_2d(nyfA,nxfA,src2D,src2D, 1,FFTW_MEASURE);
         r2 = fftwf_plan_dft_2d(nyfA,nxfA,rec2D,rec2D, 1,FFTW_MEASURE);
         getWavenumbers(nxfA, dxA, kx2A);
         getWavenumbers(nyfA, dyA, ky2A);
         get_rtc_(&t2); tPlan += t2-t1;


         // Interpolate/copy source and receiver slices from src/rec to src2D/rec2D
         get_rtc_(&t1);
         if ( (nxfA==nxf) && (nyfA==nyf)) {
            memcpy(src2D,       data->src[iwH][0], nxf*nyf*sizeof(float complex));
            memcpy(rec2D,       data->rec[iwH][0], nxf*nyf*sizeof(float complex));
            memcpy(taperXYA[0], taperXY[0],        nxf*nyf*sizeof(float)        );
         } else {
            owmg_interp_down(nxA, nyA, nxfA, nyfA, nxf, nyf,
                  dxA, dyA, data->dx, data->dy,
                  src2D, data->src[iwH][0],
                  rec2D, data->rec[iwH][0],
                  taperXYA[0], taperXY[0]);
         }
         get_rtc_(&t2); tInterpDown += t2-t1;
      }

      // Copy velocity slice - replace with average mediums
      get_rtc_(&t1);
      if ( (nxfA==nxf) && (nyfA==nyf) ) {
         // Copy first nzA
         memcpy(vP2D[0], data->vPCube[iz][0], nxf*nyf*sizeof(float));
         if ( nzA > 1 ) {
            // Add all other nzA
            for ( int i=1; i<nzA; i++ ) {
               for ( ix=0; ix<nxf*nyf; ix++ ) {
                  vP2D[0][ix] += data->vPCube[iz+i][0][ix];
               }
            }
            // Normalize
            float r = 1.0f/(float)nzA;
            for ( ix=0; ix<nxf*nyf; ix++ ) {
               vP2D[0][ix] *= r;
            }
         }
      } else {
         owmg_average(nxfA, nyfA, nzA, nxf, nyf, dxA, dyA, data->dx, data->dy,
               data->avgList, vP2D, &data->vPCube[iz]);
      }
      get_rtc_(&t2); tModel += t2-t1;

      // Minimum and maximum velocity of effective slice
      vMin=1.0f/maxval(vP2D[0], nxfA*nyfA);
      vMax=1.0f/minval(vP2D[0], nxfA*nyfA);

      // Perturbation (relative vmin)
      vP = (vMax-vMin)/vMin;

      // Choose PS propagator with tiny lateral perturbations
      propagatorA = (vP < 0.01) ? PS : data->propagator;


      get_rtc_(&t1);
      if (data->medium==ISO) {
	if (propagatorA==FFD3) {
	  vB=0.99f*vMin;
	  memIsoFFD3 *mem = (memIsoFFD3 *)data->mem;
	  iso_ffd3_co( vP2D[0], vB, nxfA, nyfA, mem->a1, mem->b1, mem->a2 );
	} else if (propagatorA==FFD2) {

	} else if (propagatorA==SSF) {
	  vB=vMin;
	} else if (propagatorA==PS) {
	  vB=vMin;
	}

      } else if (data->medium==VTI) {
	if (propagatorA==FFD3) {

	} else if (propagatorA==FFD2) {

	} else if (propagatorA==SSF) {

	} else if (propagatorA==PS) {
	}

      } else if (data->medium==TTI) {
	if (propagatorA==FFD3) {

	} else if (propagatorA==FFD2) {

	} else if (propagatorA==SSF) {

	} else if (propagatorA==PS) {
	}
      
      }
      
      k0=w/vB;
      get_rtc_(&t2); tCoeff += t2-t1;



      get_rtc_(&t1);
      // *****************************
      // ***    Isotropic model    ***
      // *****************************
      if (data->medium==ISO) {
	switch (propagatorA) {
	case FFD3:
	  get_rtc_(&t11);//T
	  iso_ps_ud(rec2D, src2D, nxfA, nyfA, kx2A, ky2A, k0, dzA, r1, s1, r2, s2);
	  get_rtc_(&t12); tPS += t12-t11; get_rtc_(&t11);//T
	  ssf_ud(rec2D, src2D, nxfA, nyfA, vP2D[0], vB, dzA, w );
	  get_rtc_(&t12); tSSF += t12-t11;  get_rtc_(&t11);//T
	  iso_ffd3_ud( (float complex *)src2D, (float complex *)rec2D, vP2D[0], (memIsoFFD3 *)data->mem,
		       w, vB, dzA, dxA, dyA, nxfA, nyfA, time );
	  get_rtc_(&t12); tFFD3 += t12-t11;//T
	  break;
	case PS:
	  get_rtc_(&t11);
	  iso_ps_ud(rec2D, src2D, nxfA, nyfA, kx2A, ky2A, k0, dzA, r1, s1, r2, s2);
	  get_rtc_(&t12); tPS += t12-t11;
	  break;
	case SSF:
	  get_rtc_(&t11);
	  iso_ps_ud(rec2D, src2D, nxfA, nyfA, kx2A, ky2A, k0, dzA, r1, s1, r2, s2);
	  get_rtc_(&t12); tPS += t12-t11; get_rtc_(&t11);
	  ssf_ud(rec2D, src2D, nxfA, nyfA, vP2D[0], vB, dzA, w );
	  get_rtc_(&t12); tSSF += t12-t11;
	  break;
	case FFD2:
	  break;
	}
      }
      get_rtc_(&t2); tProp += t2-t1;

      // Taper result
      get_rtc_(&t1);
      for (ix=0; ix<nxfA*nyfA; ix++) {
	rec2D[ix] *= taperXYA[0][ix];
	src2D[ix] *= taperXYA[0][ix];
      }
      get_rtc_(&t2); tTaper += t2-t1;

      // Take copy of previous wavefield if needed
      get_rtc_(&t1);
      if (nzA>1) {
	memcpy(srcPrev, data->src[iwH][0], nxf*nyf*sizeof(float complex));
	memcpy(recPrev, data->rec[iwH][0], nxf*nyf*sizeof(float complex));
      }
      get_rtc_(&t2); tCopy += t2-t1;

      // Copy/interpolate from rec2D/src2D to rec/src
      get_rtc_(&t1);
      if ( (nxfA==nxf) && (nyfA==nyf)) {
         memcpy(data->src[iwH][0], src2D, nxf*nyf*sizeof(float complex));
         memcpy(data->rec[iwH][0], rec2D, nxf*nyf*sizeof(float complex));
      } else {
         owmg_interp_up(data->nx, data->ny, nxfA, nyfA, nxf, nyf,
               dxA, dyA, data->dx, data->dy,
               src2D, data->src[iwH][0],
               rec2D, data->rec[iwH][0]);
      }
      get_rtc_(&t2); tInterpUp += t2-t1;


      // Imaging condition and interpolation
      {
	// Image at last slice location (nzA)
	get_rtc_(&t1);
	float *rec1Df = (float *)data->rec[iwH][0];
	float *src1Df = (float *)data->src[iwH][0];
	for ( iy=0; iy<data->ny; iy++ ) {
	  for ( ix=0; ix<data->nx; ix++ ) {
	    data->imageCubeUD[iz+nzA-1][iy][ix] += rec1Df[2*(iy*nxf+ix)]*src1Df[2*(iy*nxf+ix)]
	      + rec1Df[2*(iy*nxf+ix)+1]*src1Df[2*(iy*nxf+ix)+1];
	    data->imageCubeDD[iz+nzA-1][iy][ix] += src1Df[2*(iy*nxf+ix)]*src1Df[2*(iy*nxf+ix)]
	      + src1Df[2*(iy*nxf+ix)+1]*src1Df[2*(iy*nxf+ix)+1];
	  }
	}
	get_rtc_(&t2); tImage1 += t2-t1;

	// Interpolate between
	get_rtc_(&t1);
	if (nzA>1) {
	  for ( int i=1; i<nzA; i++ ) {
	    float k1 = (float)i/(float)nzA;
	    float k2 = 1.0f - k1;
	    //fprintf(stderr, "Weights: k1=%f, k2=%f\n", k1, k2);
	    for ( iy=0; iy<data->ny; iy++ ) {
	      for ( ix=0; ix<data->nx; ix++ ) {
		data->imageCubeUD[iz+i-1][iy][ix] += 
		  (k2*recPrev[2*(iy*nxf+ix)] + k1*rec1Df[2*(iy*nxf+ix)])*(k2*srcPrev[2*(iy*nxf+ix)] + k1*src1Df[2*(iy*nxf+ix)])
		  + (k2*recPrev[2*(iy*nxf+ix)+1] + k1*rec1Df[2*(iy*nxf+ix)+1])*(k2*srcPrev[2*(iy*nxf+ix)+1] +k1*src1Df[2*(iy*nxf+ix)+1]);
		data->imageCubeDD[iz+i-1][iy][ix] += 
		  (k2*srcPrev[2*(iy*nxf+ix)] + k1*src1Df[2*(iy*nxf+ix)])*(k2*srcPrev[2*(iy*nxf+ix)] + k1*src1Df[2*(iy*nxf+ix)])
		  + (k2*srcPrev[2*(iy*nxf+ix)+1] + k1*src1Df[2*(iy*nxf+ix)+1])*(k2*srcPrev[2*(iy*nxf+ix)+1] +k1*src1Df[2*(iy*nxf+ix)+1]);
	      }
	    }
	  }
	}
	get_rtc_(&t2); tImage2 += t2-t1;
      }

      

      iz += nzA;
      nxA_prev = nxA;
      nyA_prev = nyA;
    }

  } // end frequency loop over nz

  get_rtc_(&tEnd);

  fprintf(stderr, "Init time          : %f\n", (double)tInit/res);
  fprintf(stderr, "Plan time          : %f\n", (double)tPlan/res);
  fprintf(stderr, "Interp down time   : %f\n", (double)tInterpDown/res);
  fprintf(stderr, "Model average time : %f\n", (double)tModel/res);
  fprintf(stderr, "    init time          : %f\n", (double)tModInit/res);
  fprintf(stderr, "    run time           : %f\n", (double)tModRun/res);
  fprintf(stderr, "Coefficient time   : %f\n", (double)tCoeff/res);
  fprintf(stderr, "Propagation time   : %f\n", (double)tProp/res);
  fprintf(stderr, "    PS   time          : %f\n", (double)tPS/res);
  fprintf(stderr, "    SSF  time          : %f\n", (double)tSSF/res);
  fprintf(stderr, "    FFD3 time          : %f\n", (double)tFFD3/res);
  if (data->propagator == FFD3) {
    fprintf(stderr, "        FFD3 conj 1       : %f\n", (double)time->tConj1/res);
    fprintf(stderr, "        FFD3 X M1         : %f\n", (double)time->tXM1/res);
    fprintf(stderr, "        FFD3 X boundary 1 : %f\n", (double)time->tXBound1/res);
    fprintf(stderr, "        FFD3 X Tri 1      : %f\n", (double)time->tXTri1/res);
    fprintf(stderr, "        FFD3 X M2         : %f\n", (double)time->tXM2/res);
    fprintf(stderr, "        FFD3 X boundary 2 : %f\n", (double)time->tXBound2/res);
    fprintf(stderr, "        FFD3 X Tri 2      : %f\n", (double)time->tXTri2/res);
    fprintf(stderr, "        FFD3 Y M1         : %f\n", (double)time->tYM1/res);
    fprintf(stderr, "        FFD3 Y boundary 1 : %f\n", (double)time->tYBound1/res);
    fprintf(stderr, "        FFD3 Y Tri 1      : %f\n", (double)time->tYTri1/res);
    fprintf(stderr, "        FFD3 Y M2         : %f\n", (double)time->tYM2/res);
    fprintf(stderr, "        FFD3 Y boundary 2 : %f\n", (double)time->tYBound2/res);
    fprintf(stderr, "        FFD3 Y Tri 2      : %f\n", (double)time->tYTri2/res);
    fprintf(stderr, "        FFD3 conj 2       : %f\n", (double)time->tConj2/res);
    free(time);
  }
  fprintf(stderr, "Taper time         : %f\n", (double)tTaper/res);
  fprintf(stderr, "Copy time          : %f\n", (double)tCopy/res);
  fprintf(stderr, "Interp up time     : %f\n", (double)tInterpUp/res);
  fprintf(stderr, "Image time (ca)    : %f\n", (double)(tImage1+tImage2)/res);
  fprintf(stderr, "    Image 1 time       : %f\n", (double)tImage1/res);
  fprintf(stderr, "    Image 2 time       : %f\n", (double)tImage2/res);
  fprintf(stderr, "Total time         : %f\n", (double)(tEnd-tStart)/res);

  fftwf_destroy_plan(r1);
  fftwf_destroy_plan(s1);
  fftwf_destroy_plan(r2);
  fftwf_destroy_plan(s2);
  fftwf_free(rec2D);
  fftwf_free(src2D);
  freefloat2(vP2D);
  freefloat2(taperXY);
  freefloat2(taperXYA);
  freefloat1(srcPrev);
  freefloat1(recPrev);
  freefloat1(kx2A);
  freefloat1(ky2A);


}


void owmg_finalize(owmgData *data)
{

  for ( int i=0; i<data->nxf*data->nyf; i++) {
    if ( data->avgList[i] == NULL ) {
      free(data->avgList[i]);
    }
  }

  freefloat3(data->imageCubeUD);
  freefloat3(data->imageCubeDD);
  freefloatcomplex3(data->src);
  freefloatcomplex3(data->rec);
  freefloat3(data->vPCube);
  freefloat3(data->eCube);
  freefloat3(data->dCube);

  if (data->medium==ISO) {
    if (data->propagator==FFD3) {
      //free_iso_ffd3(data->mem); //Add TODO
    }
  } else if (data->medium==VTI) {
    if (data->propagator==FFD3) {
      
    }
  } else if (data->medium==TTI) {
    if (data->propagator==FFD3) {
    }
  }

  return;
}


void owmg_interp_down(int nxA, int nyA, int nxfA, int nyfA, int nxf, int nyf,
		      float dxA, float dyA, float dx, float dy,
		      float complex *srcA, float complex *src,
		      float complex *recA, float complex *rec,
		      float *taperXYA, float  *taperXY) {
  
  int ixy, ix, iy, ixOrg, iyOrg;

  // Zero memory area in use
  for ( ixy=0; ixy<nxfA*nyfA; ixy++ ) {
    srcA[ixy]     = 0.0f + 0*I;
    recA[ixy]     = 0.0f + 0*I;
    taperXYA[ixy] = 0.0f;
  }
  
  if ( (nxfA==nxf)||(nyfA==nyf) ) {
    fprintf(stderr, "Warning in interp_down: possible overflow, nxfA=%i, nyfA=%i\n", nxfA, nyfA);
  }
  if ( (nxfA>nxf)||(nyfA>nyf) ) {
    fprintf(stderr, "Error in interp_down: nxfA/nyfA too large, nxfA=%i, nyfA=%i\n", nxfA, nyfA);
  }

  // Fill vectors with linear weights
  ixy=0;
  float rx, ry;
  for ( iy=0; iy<nyA; iy++ ) {
    iyOrg=floorf((iy*dyA)/dy);
    ry=((iy*dyA) - iyOrg*dy)/dy;
    for ( ix=0; ix<nxA; ix++ ) {
      ixOrg=floorf((ix*dxA)/dx);
      rx=((ix*dxA) - ixOrg*dx)/dx;

      if (rx<0.0||rx>1.0||ry<0.0||ry>1.0) {
	fprintf(stderr, "Error in interp_down: rx or ry out of bounds, rx=%f, ry=%f\n", rx, ry);
	return;
      }

      if ((iyOrg+1)*nxf +ixOrg+1 >= (nxf+1)*(nyf+1)) {
	fprintf(stderr, "Error in interp_down: (iyOrg+1)*nxf +ixOrg+1 out of bounds, iyOrg=%i, ixOrg=%i, nxf=%i\n",
		iyOrg, ixOrg, nxf);
	fprintf(stderr, "rx=%f, ry=%f\n", rx, ry);
	return;
      }

      srcA[iy*nxfA+ix]=
	 (1-rx)*(1-ry)*src[ iyOrg   *nxf +ixOrg  ]
	+   rx *(1-ry)*src[ iyOrg   *nxf +ixOrg+1]
	+(1-rx)*   ry *src[(iyOrg+1)*nxf +ixOrg  ]
	+   rx *   ry *src[(iyOrg+1)*nxf +ixOrg+1];

      recA[iy*nxfA+ix]=
	 (1-rx)*(1-ry)*rec[ iyOrg   *nxf +ixOrg  ]
	+   rx *(1-ry)*rec[ iyOrg   *nxf +ixOrg+1]
	+(1-rx)*   ry *rec[(iyOrg+1)*nxf +ixOrg  ]
	+   rx *   ry *rec[(iyOrg+1)*nxf +ixOrg+1];

      taperXYA[iy*nxfA+ix]=
	 (1-rx)*(1-ry)*taperXY[ iyOrg   *nxf +ixOrg  ]
	+   rx *(1-ry)*taperXY[ iyOrg   *nxf +ixOrg+1]
	+(1-rx)*   ry *taperXY[(iyOrg+1)*nxf +ixOrg  ]
	+   rx *   ry *taperXY[(iyOrg+1)*nxf +ixOrg+1];
    }
  }
}

void owmg_interp_up(int nx, int ny, int nxfA, int nyfA, int nxf, int nyf,
		    float dxA, float dyA, float dx, float dy,
		    float complex *srcA, float complex *src,
		    float complex *recA, float complex *rec) {
  
  int ixy, ix, iy, ixA, iyA;

  // Zero memory area in use
  for ( ixy=0; ixy<nxf*nyf; ixy++ ) {
    src[ixy] = 0.0f+0*I;
    rec[ixy] = 0.0f+0*I;
  }
  
  // Fill vectors with linear weights
  ixy=0;
  float rx, ry;

  if ( (nxfA==nxf)||(nyfA==nyf) ) {
    fprintf(stderr, "Warning in interp_up: possible overflow, nxfA=%i, nyfA=%i\n", nxfA, nyfA);
  }
  if ( (nxfA>nxf)||(nyfA>nyf) ) {
    fprintf(stderr, "Error in interp_up: nxfA/nyfA too large, nxfA=%i, nyfA=%i\n", nxfA, nyfA);
  }


  for ( iy=0; iy<ny; iy++ ) {
    iyA=floorf((iy*dy)/dyA);
    ry=((iy*dy) - iyA*dyA)/dyA;
    for ( ix=0; ix<nx; ix++ ) {
      ixA=floorf((ix*dx)/dxA);
      rx=((ix*dx) - ixA*dxA)/dxA;

      if (rx<-0.0001||rx>1.0001||ry<-0.0001||ry>1.0001) {
	fprintf(stderr, "Error in interp_up: rx or ry out of bounds, rx=%f, ry=%f\n", rx, ry);
	fprintf(stderr, "ix=%i, dx=%f, dxA=%f, ixA=%i\n", ix, dx, dxA, ixA);
	return;
      }

      if ((iyA+1)*nxfA +ixA+1 >= (nxfA+1)*(nyfA+1)) {
	fprintf(stderr, "Error in interp_up: (iyA+1)*nxfA +ixA+1 out of bounds, iyA=%i, ixA=%i, nxfA=%i\n", iyA, ixA, nxfA);
	fprintf(stderr, "rx=%f, ry=%f\n", rx, ry);
	return;
      }

      src[iy*nxf+ix]=
	 (1-rx)*(1-ry)*srcA[ iyA   *nxfA +ixA  ]
	+   rx *(1-ry)*srcA[ iyA   *nxfA +ixA+1]
	+(1-rx)*   ry *srcA[(iyA+1)*nxfA +ixA  ]
	+   rx *   ry *srcA[(iyA+1)*nxfA +ixA+1];

      rec[iy*nxf+ix]=
	 (1-rx)*(1-ry)*recA[ iyA   *nxfA +ixA  ]
	+   rx *(1-ry)*recA[ iyA   *nxfA +ixA+1]
	+(1-rx)*   ry *recA[(iyA+1)*nxfA +ixA  ]
	+   rx *   ry *recA[(iyA+1)*nxfA +ixA+1];

    }
  }
  
}


void owmg_average(int nxfA, int nyfA, int nzA, int nxf, int nyf,
		  float dxA, float dyA, float dx, float dy,		  
		  averageStruct **avgList, float **vP2D, float ***vPCube) {

  int ix, iy, ixA, iyA, iz, i, maxi;
  float **sumr;
  averageStruct *iAvg;
  float rx, ry;
  unsigned long long t1, t2;

  get_rtc_(&t1);
  maxi = nxf*nyf + 2*nxf*nyfA + 2*nyf*nxfA;

  if ( avgList[(nyfA-1)*nxf+nxfA-1] == NULL ) {

    avgList[(nyfA-1)*nxf+nxfA-1] = iAvg = (averageStruct *)malloc(sizeof(averageStruct));
    iAvg->iA   = allocint1(maxi);
    iAvg->iOrg = allocint1(maxi);
    iAvg->w    = allocfloat1(maxi);

    sumr = allocfloat2(nyfA, nxfA);


    for ( ix=0; ix<nxfA*nyfA; ix++ ) {
      sumr[0][ix]=0.0f;
    }


    i=0;
    for ( iy=0; iy<nyf; iy++ ) {
      for ( ix=0; ix<nxf; ix++ ) {
	
	// Index on coarse grid
	ixA=NINT(ix*dx/dxA);
	iyA=NINT(iy*dy/dyA);
	
	rx = (ixA*dxA-ix*dx)/dx;
	ry = (iyA*dyA-iy*dy)/dy;
	
	if ( (fabsf(rx) < 0.5f) && (fabsf(ry) < 0.5f) )  {
	  // x and y on boundary
	  if (rx<0) ixA--;
	  if (ry<0) iyA--;
	  if ( (ixA>=0) && (iyA>=0) && (ixA < nxfA) && (iyA < nyfA) ) {
	    iAvg->iA  [i  ]= iyA   *nxfA + ixA;
	    iAvg->iOrg[i  ]= iy    *nxf  + ix;
	    iAvg->w   [i++]=(0.5f+rx)*(0.5f+ry);
	    sumr[iyA][ixA] += (0.5f+rx)*(0.5f+ry);
	  }
	  if ( (ixA<nxfA-1) && (iyA>=0) && (iyA < nyfA) ) {
	    iAvg->iA  [i  ]= iyA   *nxfA + ixA+1;
	    iAvg->iOrg[i  ]= iy    *nxf  + ix;
	    iAvg->w   [i++]=(0.5f-rx)*(0.5f+ry);
	    sumr[iyA][ixA+1] += (0.5f-rx)*(0.5f+ry);
	  }
	  if ( (ixA>=0) && (iyA<nyfA-1) && (ixA < nxfA) ) {
	    iAvg->iA  [i  ]=(iyA+1)*nxfA + ixA;
	    iAvg->iOrg[i  ]= iy    *nxf  + ix;
	    iAvg->w   [i++]=(0.5f+rx)*(0.5f-ry);
	    sumr[iyA+1][ixA] += (0.5f+rx)*(0.5f-ry);
	  }
	  if ( (ixA<nxfA-1) && (iyA<nyfA-1) ) {
	    iAvg->iA  [i  ]=(iyA+1)*nxfA + ixA+1;
	    iAvg->iOrg[i  ]= iy    *nxf  + ix;
	    iAvg->w   [i++]=(0.5f-rx)*(0.5f-ry);
	    sumr[iyA+1][ixA+1] += (0.5f-rx)*(0.5f-ry);
	  }
	  
	} else if ( fabsf(rx) < 0.5f )  {
	  // x on boundary
	  if (rx<0) ixA--;
	  if ( (ixA>=0) && (ixA < nxfA) && (iyA < nyfA) ) {
	    iAvg->iA  [i  ]=iyA*nxfA + ixA;
	    iAvg->iOrg[i  ]=iy *nxf  + ix;
	    iAvg->w   [i++]=(0.5f+rx);
	    sumr[iyA][ixA] += (0.5f+rx);
	  }
	  if ( (ixA<nxfA-1)  && (iyA < nyfA) ){
	    iAvg->iA  [i  ]=iyA*nxfA + ixA+1;
	    iAvg->iOrg[i  ]=iy *nxf  + ix;
	    iAvg->w   [i++]=(0.5f-rx);
	    sumr[iyA][ixA+1] += (0.5f-rx);
	  }
	  
	} else if ( fabsf(ry) < 0.5f )  {
	  // y on boundary
	  if (ry<0) iyA--;
	  if ( (iyA>=0) && (ixA < nxfA) && (iyA < nyfA) ){
	    iAvg->iA  [i  ]= iyA   *nxfA + ixA;
	    iAvg->iOrg[i  ]= iy    *nxf  + ix;
	    iAvg->w   [i++]=(0.5f+ry);
	    sumr[iyA][ixA] += (0.5f+ry);
	  }
	  if ( (iyA<nyfA-1) && (ixA < nxfA) ) {
	    iAvg->iA  [i  ]=(iyA+1)*nxfA + ixA;
	    iAvg->iOrg[i  ]= iy    *nxf  + ix;
	    iAvg->w   [i++]=(0.5f-ry);
	    sumr[iyA+1][ixA] += (0.5f-ry);
	  }
	  
	} else {
	  // interior point
	  if ( (ixA < nxfA) && (iyA < nyfA) ){
	    iAvg->iA  [i  ]=iyA*nxfA+ixA;
	    iAvg->iOrg[i  ]=iy*nxf+ix;
	    iAvg->w   [i++]=1.0f;
	    sumr[iyA][ixA] += 1.0f;
	  }
	}
      }
    }
    
    // Normalize weights
    for ( ix=0; ix<i; ix++ ) {
      iAvg->w[ix] /= sumr[0][iAvg->iA[ix]];
    }
    
    if ( i >= maxi ) {
      fprintf(stderr, "i out of bounds, i=%i, max i=%i\n", i, maxi);
    }
    iAvg->i=i;
  } else {
    iAvg = avgList[(nyfA-1)*nxf+nxfA-1];
  }
  get_rtc_(&t2); tModInit += t2-t1;
  

  get_rtc_(&t1);
  for ( ix=0; ix<nxfA*nyfA; ix++ ) {
    vP2D[0][ix]=0.0f;
  }
  float w = 1.0f/(float)nzA;
  for ( iz=0; iz<nzA; iz++ ) {
    for ( ix=0; ix<iAvg->i; ix++ ) {
      vP2D[0][iAvg->iA[ix]] += w*iAvg->w[ix]*vPCube[iz][0][iAvg->iOrg[ix]];
    }
  }
  get_rtc_(&t2); tModRun += t2-t1;

  
}

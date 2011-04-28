#ifndef STATOIL_PROPAGATOR
#define STATOIL_PROPAGATOR

#include </usr/include/complex.h>
#include <fftw3.h>
#ifdef _cplusplus
#include <cmath>
#else
#include <math.h>
#endif
#include "tools.h"
#include "rtc.h"

#define PI (3.141592653589793)

typedef struct {
  float *a1;
  float *a2;
  float *b1;
  float complex *zav;
  float complex *zbv;
  float complex *zcv;
  float complex *zdv;
  float complex *zbnx1v;
  float complex *zbnxnv;
  float complex *zdnx1v;
  float complex *zdnxnv;
  float complex *zcc;
  float *c1;
  float *c2a;
  float *c2b;
  float *c2;
  float *ssv; 
  float *c1i; 
  float *c2r;
  float *c2i;
  float *rnza; 
  float *rnzc;
  float complex *zev;
  float complex *zfv;
  float complex *zfv2;
  float complex *den;
} memIsoFFD3;

typedef struct {
  unsigned long long tConj1;
  unsigned long long tXM1;
  unsigned long long tXBound1;
  unsigned long long tXTri1;
  unsigned long long tXM2;
  unsigned long long tXBound2;
  unsigned long long tXTri2;
  unsigned long long tYM1;
  unsigned long long tYBound1;
  unsigned long long tYTri1;
  unsigned long long tYM2;
  unsigned long long tYBound2;
  unsigned long long tYTri2;
  unsigned long long tConj2;
} timeFFD3;

// Work array allocation
memIsoFFD3 *alloc_iso_ffd3(int nx, int ny);

// High level propagator interfaces
void iso_ps_ud(fftwf_complex *rec, fftwf_complex *src, int nx, int ny, float *kx2, float *ky2, float k0,
	       float dz, fftwf_plan r1, fftwf_plan s1, fftwf_plan r2, fftwf_plan s2);
void ssf_ud( fftwf_complex *rec, fftwf_complex *src, int nx, int ny, float *vP2D, float vB, float dz, float w );
void iso_ffd3_ud( float complex *src, float complex *rec, float *v, memIsoFFD3 *mem,
		  float w, float v0, float dz, float dx, float dy, int nx, int ny, timeFFD3 *time );

// Support functions for the FFD's
void iso_ffd3_co( float *v, float v0, int nx, int ny, float *a1, float*b1, float *a2 );
void iso_ffd3_m1( float wl, float *v, float v0,
		  float complex *zav, float complex *zbv, float complex *zcv, float complex *zdv,
		  float *c1, float *c2a, float *c2b, float *c2, float *ssv, 
		  float *c1i, float *c2r, float *c2i, float *rnza, float *rnzc,
		  float dzm, int nx, int ny, float dx, float *a1, float *b1, int iupd );
void iso_ffd3_m2( float wl, float *v, float v0,
		  float complex *zav, float complex *zbv, float complex *zcv, float complex *zdv,
		  float *c2, float *ssv, float *c1i, float *c2i, float *rnza, float *rnzc,
		  float dzm, int nx, int ny, float dx, float *a2, int iupd );
void iso_ffd3_mx_boundaryX( float wl, float *v,
			    float complex *zav, float complex *zbv, float complex *zcv, float complex *zdv,
			    float complex *zbnx1v, float complex *zbnxnv, float complex *zdnx1v, float complex *zdnxnv,
			    int nx, int ny, float dx, int iupd );
void iso_ffd3_mx_boundaryY( float wl, float *v,
			    float complex *zav, float complex *zbv, float complex *zcv, float complex *zdv,
			    float complex *zbnx1v, float complex *zbnxnv, float complex *zdnx1v, float complex *zdnxnv,
			    int nx, int ny, float dx, int iupd );

void trisolver_ud_2D( float complex *p, float complex *p2, 
		      float complex *za, float complex *zb, float complex *zaa, float complex *zbb,
		      float complex *zbnx1, float complex *zbnxn, float complex *zbbnx1, float complex *zbbnxn,
		      float complex *zev, float complex *zfv, float complex *zfv2, float complex *den,
		      int nx, int ny );
void trisolver_ud_1D(  float complex *p, float complex *p2, 
		       float complex *za, float complex *zb, float complex *zaa, float complex *zbb,
		       float complex zbnx1, float complex zbnxn, float complex zbbnx1, float complex zbbnxn,
		       float complex *ze, int nx );

void getWavenumbers(int nx, float dx, float *kx2);
void getTaperXY(float **taperXY, int nxf,int nyf,int nx,int ny, float alpha, int border);

#endif

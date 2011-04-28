#ifndef STATOIL_OWMG
#define STATOIL_OWMG

#ifdef _cplusplus
#include <cstring>
#else
#include <string.h>
#endif

#include </usr/include/complex.h> //Include this only when compiled outside SU? TODO
#include "tools.h"
#include "propagators.h"
#include "pfafft.h"
#include <fftw3.h>
#include "rtc.h"

/* ************************************************************************* */

#include <pthread.h>
#include <sys/time.h>

static inline double
current_time ()
{
  struct timeval tv;

  gettimeofday (&tv, NULL);

  return ((double) tv.tv_sec) + ((double) tv.tv_usec) * 1E-6;
}

/* ************************************************************************* */

#define FALSE (0)
#define TRUE (1)

#define ISO 1
#define VTI 2
#define TTI 3

#define PS 1
#define SSF 2
#define FFD2 3
#define FFD3 4

typedef struct {
  int i;
  int *iA;
  int *iOrg;
  float *w;
} averageStruct;


typedef struct {

  // Problem definition
  int medium;
  int propagator;

  // Spatial domain
  int nx;
  int ny;
  int nxf;
  int nyf;
  int nz;
  float dz;
  float dx;
  float dy;
  float latSamplesPerWave;
  float vertSamplesPerWave;

  // Frequency domain
  int iw1;
  int iw4;
  int nwH;
  float dw;

  // Storage
  float ***imageCubeUD;
  float ***imageCubeDD;
  float complex ***src;
  float complex ***rec;

  float *vPCube;
//  float ***eCube;
//  float ***dCube;
} owmgData;


typedef struct
{ owmgData * data;
  float * vMin_iz;
  int tid;
  int nThread;
  pthread_mutex_t * mutex_update;
} thread_arg_t;

owmgData *owmg_init(char *medium, char *propagator, int nx, int ny, int nz, float dx, float dy, float dz,
		    int iw1, int iw4, int nwH, float dw, float latSamplesPerWave, float vertSamplesPerWave
                   , float * pre_set_cube);
void * owmg_propagate(void * arg);
void owmg_finalize(owmgData *data);

void owmg_interp_down(int nxA, int nyA, int nxfA, int nyfA, int nxf, int nyf,
		      float dxA, float dyA, float dx, float dy,
		      float complex *srcA, float complex *src,
		      float complex *recA, float complex *rec,
		      float *taperXYA, float  *taperXY);

void owmg_interp_up(int nx, int ny, int nxfA, int nyfA, int nxf, int nyf,
		    float dxA, float dyA, float dx, float dy,
		    float complex *srcA, float complex *src,
		    float complex *recA, float complex *rec);
void owmg_average(int nxfA, int nyfA, int nzA, int nxf, int nyf,
		  float dxA, float dyA, float dx, float dy,
		  averageStruct *iAvg);
#endif

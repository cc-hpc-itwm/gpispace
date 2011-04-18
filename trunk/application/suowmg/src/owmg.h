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

#include <pthread.h>

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

  // Interpolation
  averageStruct **avgList;

  // Storage
  float ***imageCubeUD;
  float ***imageCubeDD;
  float complex ***src;
  float complex ***rec;
  
  float ***vPCube;
  float ***eCube;
  float ***dCube;
} owmgData;

typedef struct
{ pthread_mutex_t mutex;
  pthread_cond_t cond;
  int n;
  int w;
} barrier_t;

static barrier_t * new_barrier (const int n)
{
  barrier_t * b = malloc (sizeof (barrier_t));

  if (!b)
  {
	  fprintf(stderr, "failed to allocate memory for barrier description\n");
	  exit (EXIT_FAILURE);
  }

  pthread_mutex_init (&b->mutex, NULL);
  pthread_cond_init (&b->cond, NULL);
  b->n = n;
  b->w = 0;

  return b;
}

static void free_barrier (barrier_t * b)
{
	pthread_cond_destroy (&b->cond);
	pthread_mutex_destroy (&b->mutex);
	free (b);
}

static void wait_barrier (barrier_t * b)
{
	pthread_mutex_lock (&b->mutex);
	b->w = (b->w + 1) % b->n;
	if (b->w == 0)
	{
		pthread_mutex_unlock (&b->mutex);
		pthread_cond_broadcast (&b->cond);
	}
	else
	{
		pthread_cond_wait (&b->cond, &b->mutex);
		pthread_mutex_unlock (&b->mutex);
	}
}

typedef struct
{ owmgData * data;
  float * vMin_iz;
  int tid;
  int nThread;
  barrier_t * barrier;
  pthread_mutex_t * mutex_update;
} thread_arg_t;

owmgData *owmg_init(char *medium, char *propagator, int nx, int ny, int nz, float dx, float dy, float dz, 
		    int iw1, int iw4, int nwH, float dw, float latSamplesPerWave, float vertSamplesPerWave);
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

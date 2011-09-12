#include "su.h"
#include "segy.h"
#include "owmg.h"
#include "mods.h"

#include "pc.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

/*********************** self documentation **********************/
const char *sdoc[] = {
  "                             ",
  "SUOWMG - One-Way MiGration   ",
  "                             ",
  "Works for a single shot only!",
  "                             ",
  "Expects already select part from vpcube in shmem",
  "                             ",
  NULL
};

/* Credits:
 */
/**************** end self doc ***********************************/

// Function prototypes
void addSrc (owmgData * data
		, const int iLsx
		, const int iLsy
		, const int nwH
		);
void addRecTrace (segy * tr
		, owmgData * data
		, fftwf_complex *trace, const fftwf_plan * plan_trace
		, float ** slice, const float * taperw
		, const float dx, const float dy
		, const float Lox, const float Loy
		, const int nt, const int ntf
		, const int iw1, const int nwH
		);
/* void readModel (modsFILE * mFILE */
/* 		, float *cube */
/* 		, const owmgData * data */
/*         , const int iVox, const int iVoy */
/* 		, const int Vnx, const int Vny, const int Vnz */
/* 		); */

int
main (int argc, char **argv)
{
  initargs (argc, argv);
  requestdoc (1);

    void * shmem_ptr = 0;

    {
        long  shmem_size = 0;
        char *shmem_name = 0;

    	if (!getparlong ("shmem_size", &shmem_size))
    	    err ("must specify shmem_size=");

	if (!getparstring ("shmem_name", &shmem_name))
	    err ("must specify shmem_name=");

        int err = 0;
        int fd = -1;

	fd = shm_open (shmem_name, O_RDWR, 0);

    	if (fd < 0)
    	{
    		perror("shmem open");
    		exit(-1);
    	}

    	shmem_ptr = mmap ( NULL
                         , shmem_size
                         , PROT_READ | PROT_WRITE
                         , MAP_SHARED
                         , fd
                         , 0
                         );

    	if (shmem_ptr == (void*)-1)
    	{
    		perror("mmap"); exit(-1);
    	}

    	close (fd);

	fprintf(stderr,"attached to segment %s of size %ld\n", shmem_name, shmem_size);
    }


  segy tr, trout;
  int iLsx, iLsy;
  fftwf_complex *trace;
  fftwf_plan plan_trace;
  float **slice, *taperw;
  float dx, dy, dz;
  float lx, ly;
  float f1, f2, f3, f4, fPeak, df, dw;
  float Lox, Loy;
  int Vnx, Vny, Vnz, nx, ny, nz;
  int iw1, iw2, iw3, iw4, nwH;
  int nt, ntf;

  owmgData *data;
  cwp_String key1, key2;        /* header key word from segy.h         */
  cwp_String type1, type2;      /* ... its type                        */
  int indx1, indx2;             /* ... its index                       */
  int nsegy;                    /* number of bytes in the segy         */
  Value val1, val2;             /* value of key in current gather      */
  Value valnew1, valnew2;       /* value of key in trace being treated */
  float zmax, pad, latSamplesPerWave, vertSamplesPerWave;
  char *medium, *propagator, *vPFile, *dFile, *eFile;
  modsFILE *mvP, *mE, *mD;

  /* Get parameters */
  if (!getparint ("nx", &Vnx))
    err ("must specify nx=");
  if (!getparint ("ny", &Vny))
    err ("must specify ny=");
  if (!getparint ("nz", &Vnz))
    err ("must specify nz=");
  if (!getparfloat ("dx", &dx))
    err ("must specify dx=");
  if (!getparfloat ("dy", &dy))
    err ("must specify dy=");
  if (!getparfloat ("dz", &dz))
    err ("must specify dz=");
  if (!getparfloat ("zmax", &zmax))
    zmax = 0.0f;

  if (!getparfloat ("lx", &lx))
    lx = 6000;
  if (!getparfloat ("ly", &ly))
    ly = 6000;
  if (!getparfloat ("f1", &f1))
    f1 = 4;
  if (!getparfloat ("f2", &f2))
    f2 = 7;
  if (!getparfloat ("f3", &f3))
    f3 = 20;
  if (!getparfloat ("f4", &f4))
    f4 = 26;
  if (!getparfloat ("pad", &pad))
    pad = 0.0f;
  if (!getparfloat ("latSamplesPerWave", &latSamplesPerWave))
    latSamplesPerWave = 3.5;
  if (!getparfloat ("vertSamplesPerWave", &vertSamplesPerWave))
    vertSamplesPerWave = 3.5;

  if (!getparstring ("medium", &medium))
    medium = "ISO";
  if (strcmp (medium, "ISO") && strcmp (medium, "VTI")
      && strcmp (medium, "TTI"))
    {
      err ("wrong medium=");
    }
  if (!getparstring ("propagator", &propagator))
    propagator = "PS";
  if (strcmp (propagator, "PS") && strcmp (propagator, "SSF")
      && strcmp (propagator, "FFD2") && strcmp (propagator, "FFD3"))
    {
      err ("wrong propagator=");
    }

  if (!getparstring ("vPFile", &vPFile))
    err ("must specify vPFile=");
  mvP = mods_fopen (vPFile, "r");
  if (!strcmp (medium, "VTI"))
    {
      if (!getparstring ("eFile", &eFile))
        err ("must specify eFile=");
      if (!getparstring ("dFile", &dFile))
        err ("must specify dFile=");
      mE = mods_fopen (vPFile, "r");
      mD = mods_fopen (vPFile, "r");
    }

  key1 = "sx";
  type1 = hdtype (key1);
  indx1 = getindex (key1);
  key2 = "sy";
  type2 = hdtype (key2);
  indx2 = getindex (key2);

  /* Calculate parameters */

  nsegy = gettr (&tr);
  if (!nsegy)
    err ("cant't read first trace");
  if (!tr.dt)
    err ("dt header field must be set");

  {
    int nw;

    nt = tr.ns;
    ntf = NINT (nt * (1 + pad / 100.0f));
    df = 1.0e6f / ((ntf - 1) * tr.dt);
    dw = 2.0f * (float) PI *df;
    nw = ntf / 2 + 1;
    fprintf (stderr, "nt = %i\n", nt);
    fprintf (stderr, "ntf = %i\n", ntf);

    // Initial cric. freq
    iw1 = NINT (floorf (f1 / df) + 1);
    iw2 = NINT (ceilf (f2 / df) + 1);
    iw3 = NINT (floorf (f3 / df));
    iw4 = NINT (ceilf (f4 / df));

    // Circ. freqs corrected
    iw1 = IMAX (2, iw1);
    iw2 = IMAX (iw1, iw2);
    iw4 = IMIN (nw, iw4);
    iw3 = IMIN (iw4, iw3);
    fprintf (stderr, "iw1=%i, iw2=%i, iw3=%i, iw4=%i\n", iw1, iw2, iw3, iw4);

    // Number of circ. freqs to process
    nwH = iw4 - iw1 + 1;
    fprintf (stderr, "nwH = %i\n", nwH);

    // Ricker peak frequency
    fPeak = (f3 + f2) / 2.0f;
    //fprintf(stderr, "fPeak = %f\n", fPeak);

    if (latSamplesPerWave < 2.2)
      {
        err ("latSamplesPerWave too low");
      }
    if (vertSamplesPerWave < 2.2)
      {
        err ("vertSamplesPerWave too low");
      }
    fprintf (stderr, "latSamplesPerWave  = %f\n", latSamplesPerWave);
    fprintf (stderr, "vertSamplesPerWave = %f\n", vertSamplesPerWave);

  }

  // Size of computational doamin
  nx = NINT (lx / dx) + 1;
  ny = NINT (ly / dy) + 1;
  if (zmax > 0.0)
    {
      nz = IMIN (Vnz, NINT (zmax / dz));
    }
  else
    {
      nz = Vnz;
    }

  // Run owmg_init
  data =
    owmg_init (medium, propagator, nx, ny, nz, dx, dy, dz, iw1, iw4, nwH, dw,
              latSamplesPerWave, vertSamplesPerWave, shmem_ptr);

  // Work storage
  slice = allocfloat2 (ny, nx);

  // FFTW
  trace = (fftwf_complex *) fftwf_malloc (sizeof (fftwf_complex) * ntf);
  plan_trace = fftwf_plan_dft_1d (ntf, trace, trace, -1, FFTW_MEASURE);

  // Taper omega
  taperw = allocfloat1 (nwH);
  for (int iw = 0; iw < nwH; iw++)
    {
      if ((iw + 1 >= (iw1 - iw1)) && (iw + 1 <= (iw2 - iw1)) && (iw1 != iw2))
        {
          taperw[iw] =
            0.53824f - 0.46164f * cosf ((float) PI * (iw + 1) / (iw2 - iw1));
        }
      else if ((iw + 1 >= (iw3 - iw1 + 1)) && (iw + 1 <= (iw4 - iw1 + 1))
               && (iw3 != iw4))
        {
          taperw[iw] =
            0.53824f -
            0.46164f * cosf ((float) PI * (iw + iw1 - iw3) / (iw4 - iw3) -
                             (float) PI);
        }
      else
        {
          taperw[iw] = 1.0f;
        }
      //fprintf(stderr, "%f\n", taperw[iw]);
    }

  // Regularize first trace in shot
  for (int i = 0; i < nx * ny; i++)
    slice[0][i] = -2.0;
  clearfloatcomplex3 (data->src, data->nwH, data->nyf, data->nxf);
  clearfloatcomplex3 (data->rec, data->nwH, data->nyf, data->nxf);

  // prepSrc
  {
    float sx, sy;

    if (tr.scalco)
      {
        if (tr.scalco > 0)
          {
            sx = (float) tr.sx * tr.scalco;
            sy = (float) tr.sy * tr.scalco;
          }
        else
          {
            sx = (float) tr.sx / ABS (tr.scalco);
            sy = (float) tr.sy / ABS (tr.scalco);
          }
      }
    else
      {
        sx = (float) tr.sx;
        sy = (float) tr.sy;
      }

    Lox = sx - lx * 0.5f;
    Loy = sy - ly * 0.5f;

//    iLsx = NINT ((sx - Lox)/dx);
//    iLsy = NINT ((sy - Loy)/dy);
  }

  iLsx = NINT ((lx * 0.5f) / dx);
  iLsy = NINT ((ly * 0.5f) / dy);

  addRecTrace (&tr, data, trace, &plan_trace, slice, taperw, dx, dy, Lox, Loy, nt, ntf, iw1, nwH);

  // Read models
  fprintf(stderr,"read models\n");

  const int iVox = NINT (Lox / dx);
  const int iVoy = NINT (Loy / dy);

  //  readModel (mvP, data->vPCube, data, iVox, iVoy, Vnx, Vny, Vnz);

  memset ((void *) &trout, 0, sizeof (trout));

  // Collect all traces and regularize
  gethval (&tr, indx1, &val1);
  gethval (&tr, indx2, &val2);
  while (nsegy)
    {                           /* While previous trace non-empty */
      nsegy = gettr (&tr);
      gethval (&tr, indx1, &valnew1);
      gethval (&tr, indx2, &valnew2);
      if (valcmp (type1, val1, valnew1) || valcmp (type2, val2, valnew2)
          || !nsegy)
        {
          /* Either val and valnew differ, indicating a  */
          /* new gather or nsegy is zero, indicating the */
          /* end of the traces.                          */
          fprintf (stderr, "Last trace\n");

          // Add source signature to fsrc
          addSrc (data, iLsx, iLsy, nwH);

          /* Propagate */
          fprintf (stderr,
                   "Problem size: nx=%i, ny=%i, nz=%i, nxf=%i, nyf=%i, nwH=%i\n",
                   data->nx, data->ny, data->nz, data->nxf, data->nyf,
                   data->nwH);

          double t = -current_time ();

#ifndef NTHREAD
#define NTHREAD 4
#endif

          const int nThread = NTHREAD;

          pthread_attr_t attr;
          pthread_attr_init (&attr);
          pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

          thread_arg_t arg[nThread];
          pthread_t thread[nThread];

          float *vMin_iz = allocfloat1 (data->nz);

	  {
	    /*     const int first = ( tid      * data->nz + nThread - 1) / nThread; */
	    /*     const int last  = ((tid + 1) * data->nz + nThread - 1) / nThread; */

	    const int first = 0;
	    const int last = data->nz;

	    // Convert from velocity to slowness and calculate vMin
	    for (int iz = first; iz < last; ++iz)
	      {
//		vMin_iz[iz] = data->vPCube[iz][0][0];
		vMin_iz[iz] = data->vPCube[iz * data->nxf*data->nyf];

		for (int ixy = 0; ixy < data->nxf * data->nyf; ++ixy)
		  {
//		    const float val = data->vPCube[iz][0][ixy];
		    const float val = data->vPCube[iz * data->nxf*data->nyf + ixy];

		    if (val < vMin_iz[iz])
		      {
			    vMin_iz[iz] = val;
		      }

//		    data->vPCube[iz][0][ixy] = 1.0f / val;
		    data->vPCube[iz * data->nxf*data->nyf + ixy] = 1.0f / val;
		  }
	      }

	    // Zero image cubes
	    for (int ix = data->nx * data->ny * first;
		 ix < data->nx * data->ny * last; ix++)
	      {
		data->imageCubeUD[0][0][ix] = 0.0;
	      }
	    for (int ix = data->nx * data->ny * first;
		 ix < data->nx * data->ny * last; ix++)
	      {
		data->imageCubeDD[0][0][ix] = 0.0;
	      }
	  }

          pthread_mutex_t *mutex_update =
	    malloc (data->nz * sizeof (pthread_mutex_t));

          if (mutex_update == 0)
            {
              fprintf (stderr, "cannot allocate mutex-array\n");
              exit (EXIT_FAILURE);
            }

          for (int j = 0; j < data->nz; ++j)
            {
              pthread_mutex_init (mutex_update + j, NULL);
            }


          for (int tid = 1; tid < nThread; ++tid)
            {
              arg[tid].data = data;
              arg[tid].vMin_iz = vMin_iz;
              arg[tid].tid = tid;
              arg[tid].nThread = nThread;
              arg[tid].mutex_update = mutex_update;

              pthread_create (thread + tid, &attr, owmg_propagate, arg + tid);
            }

          arg[0].data = data;
          arg[0].vMin_iz = vMin_iz;
          arg[0].tid = 0;
          arg[0].nThread = nThread;
          arg[0].mutex_update = mutex_update;

          owmg_propagate (arg + 0);

          for (int tid = 1; tid < nThread; ++tid)
            {
              pthread_join (thread[tid], NULL);
            }

          for (int j = 0; j < data->nz; ++j)
            {
              pthread_mutex_destroy (mutex_update + j);
            }

          free (mutex_update);
          freefloat1 (vMin_iz);

          fprintf (stderr, "%i time run %g\n", nThread, t + current_time ());

          /* Output result */
          memset ((void *) trout.data, 0, nt * FSIZE);
          trout.ns = (unsigned short) data->nz;
          trout.dt = (unsigned short) NINT (1000 * data->dz);

          for (int i = 0; i < data->nx * data->ny; i++)
            {
              for (int iz = 0; iz < trout.ns; iz++)
                {
                  trout.data[iz] = data->imageCubeUD[iz][0][i];
                  //trout.data[iz]=data->imageCubeUD[iz][0][i]/data->imageCubeDD[iz][0][i];
                }

              // fill tracl/tracr headers and put trace out
              trout.tracl = trout.tracr = i + 1;
              puttr (&trout);
            }

          // Regularize first trace in next shot
          if (nsegy)
            {
        	  fprintf(stderr,"PROBLEM: More than one shot on input!\n");
        	  exit (EXIT_FAILURE);
            }
        }
      else
        {
          /* still in same gather */
          fprintf (stderr, "Trace in gather\n");

          addRecTrace (&tr, data, trace, &plan_trace, slice, taperw, dx, dy, Lox, Loy, nt, ntf, iw1, nwH);
        }
    }

  fprintf (stderr, "Cleanup\n");
  fftwf_destroy_plan (plan_trace);
  fftwf_free (trace);
  fftwf_cleanup ();
  freefloat2 (slice);
  freefloat1 (taperw);

  mods_fclose (mvP);
  if (!strcmp (medium, "VTI"))
    {
      mods_fclose (mE);
      mods_fclose (mD);
    }

  owmg_finalize (data);

  return (CWP_Exit ());
}

void
addRecTrace (segy * tr
		, owmgData * data
		, fftwf_complex * trace, const fftwf_plan * plan_trace
		, float ** slice, const float * taperw
		, const float dx, const float dy
		, const float Lox, const float Loy
		, const int nt, const int ntf
		, const int iw1, const int nwH
		)
{
float gx, gy;
  // Apply scalco
  if (tr->scalco)
    {
      if (tr->scalco > 0)
        {
          gx = (float) tr->gx * tr->scalco;
          gy = (float) tr->gy * tr->scalco;
        }
      else
        {
          gx = (float) tr->gx / ABS (tr->scalco);
          gy = (float) tr->gy / ABS (tr->scalco);
        }
    }
  else
    {
      gx = (float) tr->gx;
      gy = (float) tr->gy;
    }

  // Regularize
  int ix = NINT ((gx - Lox) / dx);
  int iy = NINT ((gy - Loy) / dy);
  if ((ix >= 0) && (ix < data->nx) && (iy >= 0) && (iy < data->ny))
    {
      // tmpx = distance from grid point
      float dist =
        powf (gx - ix * dx - Lox, 2.0f) + powf (gy - iy * dy - Loy, 2.0f);
      if (dist < slice[iy][ix] || slice[iy][ix] < -1.0)
        {
          // Closest to the grid point
          slice[iy][ix] = dist;
          for (int i = 0; i < ntf; i++)
            {
              trace[i] = 0.0;
            }
          for (int i = 0; i < nt; i++)
            {
              trace[i] = tr->data[i];
            }
          fftwf_execute (*plan_trace);
          for (int iw = 0; iw < nwH; iw++)
            {
              data->rec[iw][iy][ix] = (trace[iw1 + iw] * taperw[iw]);
            }
        }
    }

}


void
addSrc (owmgData * data
		, const int iLsx
		, const int iLsy
		, const int nwH
		)
{

  for (int iy = 0; iy < data->nyf; iy++)
    {
      for (int ix = 0; ix < data->nxf; ix++)
        {
          if (ix == iLsx && iy == iLsy)
            {
              // Trace through source point
              for (int iw = 0; iw < nwH; iw++)
                {
                  data->src[iw][iy][ix] = 1.0f;
                }
            }
          else
            {
              for (int iw = 0; iw < nwH; iw++)
                {
                  data->src[iw][iy][ix] = 0.0f;
                }
            }
        }
    }
}

/* void */
/* readModel ( modsFILE * mFILE */
/*           , float *cube */
/*           , const owmgData * data */
/*           , const int iVox, const int iVoy */
/*           , const int Vnx, const int Vny, const int Vnz */
/*           ) */
/* { */
/*   const int nxf = data->nxf; */
/*   const int nyf = data->nyf; */
/*   const int nz = data->nz; */

/*   float *nvec = malloc (nxf * Vnz * sizeof (float)); */

/*   // Calculate padding (extrapolation) */
/*   const int padx1 = IMAX (0, -iVox); */
/*   const int padx2 = IMAX (0, iVox + nxf - Vnx); */
/*   const int pady1 = IMAX (0, -iVoy); */
/*   const int pady2 = IMAX (0, iVoy + nyf - Vny); */

/*   for (int ix = 0; ix < nxf * nyf * nz; ++ix) */
/*     cube[ix] = -2.0; */

/*   // Loop over y */
/*   for (int iy = iVoy + pady1; iy <= iVoy + nyf - 1 - pady2; iy++) */
/*     { */

/*       // Read from disk */
/*       for (int ix = 0; ix < nxf * Vnz; ix++) */
/*         nvec[ix] = -1.0; */

/*       mods_fread (mFILE, nvec + padx1 * Vnz, iy * Vnx * Vnz + (iVox + padx1) * Vnz, */
/*                  (nxf - padx2 - padx1) * Vnz); */

/*       // Pad in x-direction (trace by trace) */
/*       if (padx1 > 0) */
/*         { */
/*           for (int ix = 0; ix < padx1; ix++) */
/*             { */
/*               for (int iz = 0; iz < nz; iz++) */
/*                 { */
/*                   nvec[Vnz * ix + iz] = nvec[Vnz * padx1 + iz]; */
/*                 } */
/*             } */
/*         } */
/*       if (padx2 > 0) */
/*         { */
/*           for (int ix = nxf - padx2; ix < nxf; ix++) */
/*             { */
/*               for (int iz = 0; iz < nz; iz++) */
/*                 { */
/*                   nvec[Vnz * ix + iz] = nvec[Vnz * (nxf - padx2 - 1) + iz]; */
/*                 } */
/*             } */
/*         } */

/*       // Transpose and place in cube */
/*       for (int ix = 0; ix < nxf; ix++) */
/*         { */
/*           for (int iz = 0; iz < nz; iz++) */
/*             { */
/*               cube[(iz * nyf + iy - iVoy) * nxf + ix] = nvec[Vnz * ix + iz]; */
/*             } */
/*         } */
/*     } */

/*   if (pady1 > 0) */
/*     { */
/*       for (int iz = 0; iz < nz; iz++) */
/*         { */
/*           for (int iy = 0; iy < pady1; iy++) */
/*             { */
/*               for (int ix = 0; ix < nxf; ix++) */
/*                 { */
/*                   cube[(iz * nyf + iy) * nxf + ix] */
/*                     = cube[(iz * nyf + pady1) * nxf + ix]; */
/*                 } */
/*             } */
/*         } */
/*     } */

/*   if (pady2 > 0) */
/*     { */
/*       for (int iz = 0; iz < nz; iz++) */
/*         { */
/*           for (int iy = nyf - pady2; iy < nyf; iy++) */
/*             { */
/*               for (int ix = 0; ix < nxf; ix++) */
/*                 { */
/*                   cube[(iz * nyf + iy) * nxf + ix] */
/*                     = cube[(iz * nyf + nyf - pady2 - 1) * nxf + ix]; */
/*                 } */
/*             } */
/*         } */
/*     } */

/*   free (nvec); */

/*   return; */
/* } */

/***************************************************************************
                          TravTimeTab.hpp  -  description
*/

/**
    Doxygen Style
    Multi-Line Documentation

    Manages the travel time tables
    Usage:
    1.) Instantiate the TravTimeTab object by passing an
	instance of the MigrationJob class
    2.) Load the travel time table from disk for a given trace and
        a given subvolume into local memory by calling
        LoadTT. The interpolation of the travel time table
    3.) During migration: loop over x,y coordinates
	of the subvolume.
        Call Init_ix
             Init_iy
    4.) In order to obtain the traveltimes for a complete depth
        column, call Init_iz and GetTT
**/

/*                           -------------------
    begin                :
    copyright            : (C) 2010 by Daniel Grünewald
    email                : Daniel.Gruenewald@itwm.fraunhofer.de
 ***************************************************************************/

#ifndef TRAVTIMETAB_H
#define TRAVTIMETAB_H

// user defined includes
#include "filehandler/ttfilehandler.h"
#include "ttvmmemhandler.h"
//#include "ttmemhandler.h"
//#include "synthgenerator.h"
#include "structures/migrationjob.h"
#include "structures/grid3d.h"
#include "structures/grid2d.h"
#include "structures/point3d.h"
#include "structures/mod.h"
#include "structures/recsig.h"
#include "include/migtypes.h"
#include "include/defs.h"

#include "MigSubVol.hpp"
#include "TraceData.hpp"

#ifdef __SSE__
#include <xmmintrin.h>
#endif

// standard includes
#include <string.h>

#include <fvm-pc/pc.hpp>

/**
  *@author Daniel Grünewald doxygen stale
  */

class TravTimeTab {

// public methods
 public:

  // constructor/destructor
  TravTimeTab();
  TravTimeTab(const MigrationJob& MigJob);
  TravTimeTab(const MigrationJob& MigJob,int NThreads, char *);
  TravTimeTab(volatile char * _pTTData, const MigrationJob& MigJob);
  ~TravTimeTab();

  // member functions
  bool LoadTT(TraceData& Trace,MigSubVol3D& SubVol,const MigrationJob &Job,int mtid, const fvmAllocHandle_t);

  bool Init(const MigrationJob &Job,
            const char* Name, const MOD& Offx, const MOD& Offy,
	    const MOD& Mptx, const MOD& Mpty,const int firstxpindex,
           const int Nxpoints, const int firstypindex, const int Nypoints,int mtid, const fvmAllocHandle_t);

  void Init_ix(const int& ix);

  void Init_iy(const int& iy);

  void Init_iz(const int& Nz);

  // get traveltime information
  void GetTT(float* T, const int& Nz );

  void GetTT(float* T, float* dTdx, float* dTdy, const int& Nz );

  void GetTT(float* T, float* dTdx, float* dTdy, float* ps_plus_pr,
	     float* detQ, float* detH, const int& Nz );

  void Reset();

  volatile char * getMemPtr();
  void SetMemPtr(volatile char * _pTTData);

// public attributes
 public:

// private methods
 private:

    TravTimeTab(const TravTimeTab& T);

    TravTimeTab& operator = (const TravTimeTab& T);

    // Initialize the grid definitions for the surface and
    // the subvolume grid
    void InitGrids(const MigrationJob& MigJob);

    // Allocate the SrcRcv TT arrays for a subvolume of
    // size Nx*Ny*Nz
    void MemAlloc(const int Nx, const int Ny, const int Nz);


// private attributes
 private:

  TTFileHandler TTFile;
  TTVMMemHandler TTVMMem;
  char TTFileBaseName[1000];
  //TTMemHandler TTMem;

  // description
  short xfac;
  short yfac;
  short zfac;

  short ix0, ixf, ixc;
  short iy0, iyf, iyc;
  short iz0, izf, izc;

  // Boundary
  float dxBnd, dyBnd, dzBnd;

  // Resolution of the Velocity Model
  point3D<float> X0Vel, dxVel;
  point3D<int> NVel;

  // Number of rays per source and max length for refinement
  int g_InitAngle;
  int g_REF_LEN;

  // Time step size
  int g_MAXTSTEP;
  float g_TSTEPSIZE;
  int g_TRACINGSTEPS;

  grid2D GSrc;
  grid3D GVol;

  // coefficients for linear interpolation in the sub-surface volume
  float**** coeff;
  float*** coeffxy;
  float** coeffz;

  // Refinement factors between sub-surface volume grid and sub-surface traveltime grid
  float zfac_inv;
  myaligned( float y[8], 16);
  myaligned( float dtdx[8], 16);
  myaligned( float dtdy[8], 16);
  myaligned( float detHy[8], 16);
  myaligned( float detQy[8], 16);
  myaligned( float ps_plus_pry[8], 16);

  // coefficients for linear interpolation in the Source/Receiver position
  float txSrc, tySrc, txRcv, tyRcv;
  float txSrc_1, tySrc_1, txRcv_1, tyRcv_1;

  float* TT00;
  float* TT01;
  float* TT10;
  float* TT11;

  float* TT00dx;
  float* TT01dx;
  float* TT10dx;
  float* TT11dx;

  float* TT00dy;
  float* TT01dy;
  float* TT10dy;
  float* TT11dy;

  float* detH00;
  float* detH01;
  float* detH10;
  float* detH11;

  float* detQ00;
  float* detQ01;
  float* detQ10;
  float* detQ11;

  float* ps_plus_pr00;
  float* ps_plus_pr01;
  float* ps_plus_pr10;
  float* ps_plus_pr11;

  /// 3-dim array of runtimes
  float* Src00;
  float* Src01;
  float* Src10;
  float* Src11;
  float* Rcv00;
  float* Rcv01;
  float* Rcv10;
  float* Rcv11;

  //MemMan3D_bucket<float> SrcRcv;
  float* SrcRcv;
  float* SrcRcvLocal;
  float* SrcRcvdx;
  float* SrcRcvdxLocal;
  float* SrcRcvdy;
  float* SrcRcvdyLocal;

  float* SrcRcvdetH;

  float* SrcRcvdetQ;

  float* SrcRcvps_plus_pr;

  // vector of indicees to access the TT and coeff maps for volume interpolation
  short* izfc_vec;

  volatile char * pTTData;
  bool   MemRqst;

};

#endif

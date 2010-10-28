/***************************************************************************
                          migrationjob.h  -  description
                             -------------------
    begin                : Thu Mar 30 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

      merten | 2010-01-11
               AperAng_deg renamed to CDPAperture_deg
               TaperWidthAperture renamed to CDPApertureTaperWidth_mtr

      merten | 2010-01-06
               aperture angle changed to float and renamed to AperAng_deg

      merten | 2009-08-03
               Adapted to modified seisgrid3D class internal structure.
               SrfcGridX0/Y0 changed from float to  mod

      micheld | 2009-08-07
		parameter controlling rays per source in raytracing changed from g_RAY_MIN to g_InitAngle

      micheld | 2009-08-12
		copy-operators were lacking Offset-class and geometry descriptions

 ***************************************************************************/


#ifndef MIGRATIONJOB_H
#define MIGRATIONJOB_H


/**
  *@author Dirk Merten
  */
#include "include/migtypes.h"
#include "structures/geomdefstruct.hpp"
#include "structures/point3d.h"
#include "structures/seisgrid3d.h"
#include "structures/mod.h"
#include "structures/utm.h"
#include "structures/deg.h"

#include <string.h>

class MigrationJob {
public:
  MigrationJob();
  MigrationJob(const MigrationJob& Job);
  MigrationJob& operator=(const MigrationJob& Job);
  ~MigrationJob();

  char ProjectName[199];
  char JobName[199];
  int LogLevel;
  char LogFile[199];

  /** Definition of the reference geometry frame */
  geomdefstruct geom;

  /** origin, dimension and resolution of the migration volume in model coordinates */
  seisgrid3D MigVol;

  float Off0Vol;
  int NOffVol;
  int N0OffVol;
  int NtotOffVol;
  float dOffVol;

  // Regular Data
  // Offsets
  float Offx0;
  int NOffx;
  int N0Offx;
  int NtotOffx;
  float Offdx;

  float Offy0;
  int NOffy;
  int N0Offy;
  int NtotOffy;
  float Offdy;

  // CDPs
  float CDPx0;
  int NCDPx;
  int N0CDPx;
  int NtotCDPx;
  float CDPdx;

  float CDPy0;
  int NCDPy;
  int N0CDPy;
  int NtotCDPy;
  float CDPdy;

  /** origin, dimension and resolution of the input data in il/xl coordinates */
  int n_inlines_CDP, n_xlines_CDP;
  float d_between_inlines_CDP, d_between_xlines_CDP;
  int first_inline_num_CDP, first_xline_num_CDP;
  float first_offset;
  float d_offset;
  int n_offset;
  int n_start_use_inlines_CDP, n_start_use_xlines_CDP;
  int n_use_inlines_CDP, n_use_xlines_CDP;

  /// Travel time tables
  /// TT table Surface grid
  int n_inlines_TT_Srfc, n_xlines_TT_Srfc;
  float d_between_inlines_TT_Srfc, d_between_xlines_TT_Srfc;
  int first_inline_num_TT_Srfc, first_xline_num_TT_Srfc;


  MOD SrfcGridX0;
  MOD SrfcGridY0;

  float SrfcGriddx;
  float SrfcGriddy;

  int SrfcGridNx;
  int SrfcGridNy;

  /** origin, dimension and resolution of the TT table volume in model coordinates */
  seisgrid3D TTVol;

  /** Maximum aperture angle (in deg). Subsurface points seen from a CMP under an angle larger
      than this are neglected.
      This angle is given realitiv to the vertical, range 0..90*/
  DEG CDPAperture_deg;
  /**  Width of the taper for CDPAperture_deg.
       Taper is given and applied in the spatial distance of the corresponding surface coordinates. */
  float CDPApertureTaperWidth_mtr;

  /// File names
  char TraceFileName[1000];
  char TracePositionFileName[1000];
  char RTFileName[1000];
  char MigDirName[1000];
  char MigFileName[1000];

  /** Directory of the travel time tables */
  char TTDirName[199];
  /** Name of the travel time table master file */
  char TTCfgFileName[199];
  /** Name prefix of the travel time tables */
  char TTFilePrefix[199];

  /// File types
  FILE_ORDER TraceOrder;
  FILE_MODE TraceFileMode;
  FILE_MODE MigFileMode;

  /// Gathering mode
  GATHER_MODE GatherMode;

  /// corner, number and resolution of the velocity model
  point3D<float> X0Vel, dxVel;
  point3D<int> NVel;

  /// Size of the Boundary around Sources and Receivers
  point3D<float> dxBnd;

  /** Initial angular spreading of rays per source */
  DEG g_InitAngle;

  /** critical length for refinement (WAVEFRONTTRACER)*/
  int g_REF_LEN;
  /** Angle resolution (RAYTRACER)*/
  DEG g_DANGLE;

  /// Time step size
  int g_MAXTSTEP;
  float g_TSTEPSIZE;
  int g_TRACINGSTEPS;

  /** Flag for derivative of Input Data */
  bool frac;

  /** Flag for derivative of Input Data */
  bool anti_aliasing;

  /** Band pass Frequencies */
  float frequ1, frequ2, frequ3, frequ4;

  float clip;
  float trap;

  /** power for tpow preprocessing */
  float tpow;

  /// Slant stack information
  bool use_sstacks;
  char SlantStackFileName[1000];
  char SlantStackPositionFileName[1000];
  FILE_MODE SlantStackFileMode;
  int n_azimuths, n_inclinations;
  float f_spacing_azimuth, f_initial_azimuth, f_spacing_inclination, f_initial_inclination;
  int i_AngleSorting;
  float f_surfacevelocity;

  /// SDPA control parameters
  int tracesperbunch;
  int traceNt;
  float tracedt;
  int NSubVols;

  /// SDPA VM parameters
  int ReqVMMemSize;

  long globTTbufsizelocal;

  int locbufsize;	  // Size of the local buffers;

  int BunchMemSize;       // Size of a bunch in bytes

  int SubVolMemSize;      // Size of the subvolumina in bytes
  bool sinc_initialized;

  unsigned long shift_for_TT;
  unsigned long shift_for_Vol;

};

#endif

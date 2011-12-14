/***************************************************************************
                          migrationjob.cpp  -  description
                             -------------------
    begin                : Thu Mar 30 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/

#include "migrationjob.h"
#include <stdio.h>

MigrationJob::MigrationJob()
{
  sprintf(ProjectName, "");
  sprintf(JobName, "");
  sprintf(LogFile, "");

  LogLevel = 1;

  sprintf(TraceFileName, "");

  sprintf(MigDirName, "");
  sprintf(MigFileName, "");

  sprintf(RTFileName, "");
  sprintf(TTDirName, "");
  sprintf(TTFilePrefix, "");
  sprintf(TTCfgFileName, "");


  // Subsurface Geometry that has to be migrated
  MigVol.first_x_coord = 0;
  MigVol.first_z_coord = 0;
  MigVol.first_z_coord = -3500;
  MigVol.nx_xlines = 7000;
  MigVol.ny_inlines = 7000;
  MigVol.nz = 7000;
  MigVol.dx_between_xlines = 5;
  MigVol.dy_between_inlines = 5;
  MigVol.dz = 5;

  Offx0 = 0;
  NOffx = 1;
  Offdx = 1;

  Offy0 = 0;
  NOffy = 1;
  Offdy = 1;

  // MidPoints
  CDPx0 = 0;
  NCDPx = 1;
  CDPdx = 1;

  CDPy0 = 0;
  NCDPy = 1;
  CDPdy = 1;

  // Aperture angle
  CDPAperture_deg = 180.f;

  CDPApertureTaperWidth_mtr = 100.0f;

  use_sstacks = false;

  frequ1 = frequ2 = frequ3 = frequ4 = -1.f;

  clip = -1.f;
  trap = -1.f;

  tpow= 0.f;

  /// Modifications by Daniel Grünewald

  /// SDPA control parameters
  tracesperbunch=1;  // initialized by checkreadmigrationjob
  traceNt=0;         // initialized by checkreadmigrationjob
  tracedt=0.;        // initialized by checkreadmigrationjob
  NSubVols=0;        // initialized by checkreadmigrationjob

  /// SDPA VM parameters
  ReqVMMemSize=0;      // initialized by SDPA_init

  globTTbufsizelocal=0;// intialized by SDPA_init

  SubVolMemSize = 0;

  shift_for_Vol = 0;
  BunchMemSize=0;      // initialized by SDPA_init
  shift_for_TT = 0;


  sinc_initialized = false;

}

MigrationJob::MigrationJob(const MigrationJob& Job)
{
  memcpy(ProjectName, Job.ProjectName, 199);
  memcpy(JobName, Job.JobName, 199);
  LogLevel=Job.LogLevel;
  memcpy(LogFile, Job.LogFile, 199);

  sprintf(TraceFileName, "%s", Job.TraceFileName);
  sprintf(TracePositionFileName, "%s", Job.TracePositionFileName);
  sprintf(SlantStackFileName, "%s", Job.SlantStackFileName);
  sprintf(SlantStackPositionFileName, "%s", Job.SlantStackPositionFileName);

  sprintf(RTFileName, "%s", Job.RTFileName);

  sprintf(MigDirName, "%s", Job.MigDirName);
  sprintf(MigFileName, "%s", Job.MigFileName);

  memcpy(TTDirName, Job.TTDirName, 199);
  memcpy(TTCfgFileName, Job.TTCfgFileName, 199);
  memcpy(TTFilePrefix, Job.TTFilePrefix, 199);

  MigVol = Job.MigVol;
  geom = Job.geom;
  Off0Vol = Job.Off0Vol;
  NOffVol = Job.NOffVol;
  N0OffVol = Job.N0OffVol;
  NtotOffVol = Job.NtotOffVol;
  dOffVol = Job.dOffVol;

  geom = Job.geom;
  Off0Vol = Job.Off0Vol;
  NOffVol = Job.NOffVol;
  N0OffVol = Job.N0OffVol;
  NtotOffVol = Job.NtotOffVol;
  dOffVol = Job.dOffVol;

  Offx0 = Job.Offx0;
  NOffx = Job.NOffx;
  N0Offx = Job.N0Offx;
  NtotOffx = Job.NtotOffx;
  Offdx = Job.Offdx;

  Offy0 = Job.Offy0;
  NOffy = Job.NOffy;
  N0Offy = Job.N0Offy;
  NtotOffy = Job.NtotOffy;
  Offdy = Job.Offdy;

  CDPx0 = Job.CDPx0;
  NCDPx = Job.NCDPx;
  N0CDPx = Job.N0CDPx;
  NtotCDPx = Job.NtotCDPx;
  CDPdx = Job.CDPdx;

  CDPy0 = Job.CDPy0;
  NCDPy = Job.NCDPy;
  N0CDPy = Job.N0CDPy;
  NtotCDPy = Job.NtotCDPy;
  CDPdy = Job.CDPdy;

  n_inlines_CDP = Job.n_inlines_CDP;
  n_xlines_CDP = Job.n_xlines_CDP;
  d_between_inlines_CDP = Job.d_between_inlines_CDP;
  d_between_xlines_CDP = Job.d_between_xlines_CDP;
  first_inline_num_CDP = Job.first_inline_num_CDP;
  first_xline_num_CDP = Job.first_xline_num_CDP;
  first_offset = Job.first_offset;
  d_offset = Job.d_offset;
  n_offset = Job.n_offset;
  n_start_use_inlines_CDP = Job.n_start_use_inlines_CDP;
  n_start_use_xlines_CDP = Job.n_start_use_xlines_CDP;
  n_use_inlines_CDP = Job.n_use_inlines_CDP;
  n_use_xlines_CDP = Job.n_use_xlines_CDP;


  SrfcGridX0 = Job.SrfcGridX0;
  SrfcGridY0 = Job.SrfcGridY0;

  SrfcGriddx = Job.SrfcGriddx;
  SrfcGriddy = Job.SrfcGriddy;

  SrfcGridNx = Job.SrfcGridNx;
  SrfcGridNy = Job.SrfcGridNy;

  n_inlines_TT_Srfc = Job.n_inlines_TT_Srfc;
  n_xlines_TT_Srfc = Job.n_xlines_TT_Srfc;
  d_between_inlines_TT_Srfc = Job.d_between_inlines_TT_Srfc;
  d_between_xlines_TT_Srfc = Job.d_between_xlines_TT_Srfc;
  first_inline_num_TT_Srfc = Job.first_inline_num_TT_Srfc;
  first_xline_num_TT_Srfc = Job.first_xline_num_TT_Srfc;


  TTVol = Job.TTVol;
//   VolGridX0 = Job.VolGridX0;
//   VolGridY0 = Job.VolGridY0;
//   VolGridZ0 = Job.VolGridZ0;

//   VolGriddx = Job.VolGriddx;
//   VolGriddy = Job.VolGriddy;
//   VolGriddz = Job.VolGriddz;

//   VolGridNx = Job.VolGridNx;
//   VolGridNy = Job.VolGridNy;
//   VolGridNz = Job.VolGridNz;

//   n_inlines_TT_SubSrfc = Job.n_inlines_TT_SubSrfc;
//   n_xlines_TT_SubSrfc = Job.n_xlines_TT_SubSrfc;
//   d_between_inlines_TT_SubSrfc = Job.d_between_inlines_TT_SubSrfc;
//   d_between_xlines_TT_SubSrfc = Job.d_between_xlines_TT_SubSrfc;
//   first_inline_num_TT_SubSrfc = Job.first_inline_num_TT_SubSrfc;
//   first_xline_num_TT_SubSrfc = Job.first_xline_num_TT_SubSrfc;
//   minDepth_TT_SubSrfc = Job.minDepth_TT_SubSrfc;
//   dz_TT_SubSrfc = Job.dz_TT_SubSrfc;
//   Nz_TT_SubSrfc = Job.Nz_TT_SubSrfc;



   CDPAperture_deg = Job.CDPAperture_deg;


  TraceFileMode  = Job.TraceFileMode;
  SlantStackFileMode = Job.SlantStackFileMode;
  TraceOrder = Job.TraceOrder;
  MigFileMode = Job.MigFileMode;
  GatherMode = Job.GatherMode;

  // corner, number and resolution of the velocity model
  X0Vel = Job.X0Vel;
  dxVel = Job.dxVel;
  NVel = Job.NVel;

  // Size of the Boundary around Sources and Receivers
  dxBnd = Job.dxBnd;

  // Number of rays per source and max lenght for refinement
  g_InitAngle = Job.g_InitAngle;
  g_REF_LEN = Job.g_REF_LEN;

  // Time step size
  g_MAXTSTEP = Job.g_MAXTSTEP;
  g_TSTEPSIZE = Job.g_TSTEPSIZE;
  g_TRACINGSTEPS = Job.g_TRACINGSTEPS;

  frac = Job.frac;
  anti_aliasing = Job.anti_aliasing;


  frequ1 = Job.frequ1;
  frequ2 = Job.frequ2;
  frequ3 = Job.frequ3;
  frequ4 = Job.frequ4;

  clip = Job.clip;
  trap = Job.trap;

  tpow = Job.tpow;

  use_sstacks = Job.use_sstacks;
  n_azimuths = Job.n_azimuths;
  n_inclinations = Job.n_inclinations;
  f_spacing_azimuth = Job.f_spacing_azimuth;
  f_initial_azimuth = Job.f_initial_azimuth;
  f_spacing_inclination = Job.f_spacing_inclination;
  f_initial_inclination = Job.f_initial_inclination;
  i_AngleSorting = Job.i_AngleSorting;

  /// SDPA control parameters
  //  bunchesperpack=Job.bunchesperpack;
  tracesperbunch=Job.tracesperbunch;
  //  NpackInNet=Job.NpackInNet;
  traceNt=Job.traceNt;
  tracedt=Job.tracedt;
  NSubVols=Job.NSubVols;

  /// SDPA VM parameters
  ReqVMMemSize=Job.ReqVMMemSize;

  globTTbufsizelocal=Job.globTTbufsizelocal;

  SubVolMemSize = Job.SubVolMemSize;

  shift_for_TT = Job.shift_for_TT;

  shift_for_Vol = Job.shift_for_Vol;
  BunchMemSize=Job.BunchMemSize;

  sinc_initialized = Job.sinc_initialized;


}

MigrationJob& MigrationJob::operator=(const MigrationJob& Job)
{
  memcpy(ProjectName, Job.ProjectName, 199);
  memcpy(JobName, Job.JobName, 199);
  LogLevel=Job.LogLevel;
  memcpy(LogFile, Job.LogFile, 199);

  sprintf(TraceFileName, "%s", Job.TraceFileName);
  sprintf(TracePositionFileName, "%s", Job.TracePositionFileName);
  sprintf(SlantStackFileName, "%s", Job.SlantStackFileName);
  sprintf(SlantStackPositionFileName, "%s", Job.SlantStackPositionFileName);

  sprintf(RTFileName, "%s", Job.RTFileName);

  sprintf(MigDirName, "%s", Job.MigDirName);
  sprintf(MigFileName, "%s", Job.MigFileName);

  memcpy(TTDirName, Job.TTDirName, 199);
  memcpy(TTCfgFileName, Job.TTCfgFileName, 199);
  memcpy(TTFilePrefix, Job.TTFilePrefix, 199);

  MigVol = Job.MigVol;
  geom = Job.geom;
  Off0Vol = Job.Off0Vol;
  NOffVol = Job.NOffVol;
  N0OffVol = Job.N0OffVol;
  NtotOffVol = Job.NtotOffVol;
  dOffVol = Job.dOffVol;

  Offx0 = Job.Offx0;
  NOffx = Job.NOffx;
  N0Offx = Job.N0Offx;
  NtotOffx = Job.NtotOffx;
  Offdx = Job.Offdx;

  Offy0 = Job.Offy0;
  NOffy = Job.NOffy;
  N0Offy = Job.N0Offy;
  NtotOffy = Job.NtotOffy;
  Offdy = Job.Offdy;

  CDPx0 = Job.CDPx0;
  NCDPx = Job.NCDPx;
  N0CDPx = Job.N0CDPx;
  NtotCDPx = Job.NtotCDPx;
  CDPdx = Job.CDPdx;

  CDPy0 = Job.CDPy0;
  NCDPy = Job.NCDPy;
  N0CDPy = Job.N0CDPy;
  NtotCDPy = Job.NtotCDPy;
  CDPdy = Job.CDPdy;

  n_inlines_CDP = Job.n_inlines_CDP;
  n_xlines_CDP = Job.n_xlines_CDP;
  d_between_inlines_CDP = Job.d_between_inlines_CDP;
  d_between_xlines_CDP = Job.d_between_xlines_CDP;
  first_inline_num_CDP = Job.first_inline_num_CDP;
  first_xline_num_CDP = Job.first_xline_num_CDP;
  first_offset = Job.first_offset;
  d_offset = Job.d_offset;
  n_offset = Job.n_offset;
  n_start_use_inlines_CDP = Job.n_start_use_inlines_CDP;
  n_start_use_xlines_CDP = Job.n_start_use_xlines_CDP;
  n_use_inlines_CDP = Job.n_use_inlines_CDP;
  n_use_xlines_CDP = Job.n_use_xlines_CDP;


  SrfcGridX0 = Job.SrfcGridX0;
  SrfcGridY0 = Job.SrfcGridY0;

  SrfcGriddx = Job.SrfcGriddx;
  SrfcGriddy = Job.SrfcGriddy;

  SrfcGridNx = Job.SrfcGridNx;
  SrfcGridNy = Job.SrfcGridNy;

  use_sstacks = Job.use_sstacks;
  n_inlines_TT_Srfc = Job.n_inlines_TT_Srfc;
  n_xlines_TT_Srfc = Job.n_xlines_TT_Srfc;
  d_between_inlines_TT_Srfc = Job.d_between_inlines_TT_Srfc;
  d_between_xlines_TT_Srfc = Job.d_between_xlines_TT_Srfc;
  first_inline_num_TT_Srfc = Job.first_inline_num_TT_Srfc;
  first_xline_num_TT_Srfc = Job.first_xline_num_TT_Srfc;

  TTVol = Job.TTVol;

  CDPAperture_deg = Job.CDPAperture_deg;

  SlantStackFileMode = Job.SlantStackFileMode;
  TraceFileMode  = Job.TraceFileMode;
  TraceOrder = Job.TraceOrder;
  MigFileMode = Job.MigFileMode;
  GatherMode = Job.GatherMode;

  // corner, number and resolution of the velocity model
  X0Vel = Job.X0Vel;
  dxVel = Job.dxVel;
  NVel = Job.NVel;

  // Size of the Boundary around Sources and Receivers
  dxBnd = Job.dxBnd;

  // Number of rays per source and max lenght for refinement
  g_InitAngle = Job.g_InitAngle;
  g_REF_LEN = Job.g_REF_LEN;

  // Time step size
  g_MAXTSTEP = Job.g_MAXTSTEP;
  g_TSTEPSIZE = Job.g_TSTEPSIZE;
  g_TRACINGSTEPS = Job.g_TRACINGSTEPS;

  frac = Job.frac;
  anti_aliasing = Job.anti_aliasing;

  frequ1 = Job.frequ1;
  frequ2 = Job.frequ2;
  frequ3 = Job.frequ3;
  frequ4 = Job.frequ4;

  clip = Job.clip;
  trap = Job.trap;

  tpow = Job.tpow;

  use_sstacks = Job.use_sstacks;
  n_azimuths = Job.n_azimuths;
  n_inclinations = Job.n_inclinations;
  f_spacing_azimuth = Job.f_spacing_azimuth;
  f_initial_azimuth = Job.f_initial_azimuth;
  f_spacing_inclination = Job.f_spacing_inclination;
  f_initial_inclination = Job.f_initial_inclination;
  i_AngleSorting = Job.i_AngleSorting;

  /// SDPA control parameters
  tracesperbunch=Job.tracesperbunch;
  traceNt=Job.traceNt;
  tracedt=Job.tracedt;
  NSubVols=Job.NSubVols;

  /// SDPA VM parameters
  ReqVMMemSize=Job.ReqVMMemSize;

  globTTbufsizelocal=Job.globTTbufsizelocal;

  SubVolMemSize = Job.SubVolMemSize;
  shift_for_TT = Job.shift_for_TT;
  shift_for_Vol = Job.shift_for_Vol;
  BunchMemSize=Job.BunchMemSize;
  sinc_initialized = Job.sinc_initialized;

  return *this;
}

MigrationJob::~MigrationJob(){

}

/***************************************************************************
                          anglemigrationjob.cpp  -  description
                             -------------------
    begin                : Mon Nov 27 2006
    copyright            : (C) 2006 by Dirk Merten
    email                : merten@itwm.fhg.de
 ***************************************************************************/


/**
  *@author Dirk Merten
  */
#include "anglemigrationjob.h"

AngleMigrationJob::AngleMigrationJob()
{
  sprintf(ProjectName, "");
  sprintf(JobName, "");
  sprintf(LogFile, "");

  LogLevel = 1;

  // Rays Fan resolutions
  DANG_RAYFAN_COARSE = -1;
  DANG_RAYFAN_FINE = -1;
  
  // Number of Opening Angles
  N_PHI_OPEN = -1;
  N_GATHER = -1;

  // Number of azimuth classes
  dazimuth_deg = 180.0f;
  N_AZIMUTH_CLASSES = 1;

  // File names
  NTraceFiles = 0;
  TraceFileName = NULL;
  sprintf(TTDirName, "./");
  sprintf(TTFilePrefix, "");
  sprintf(TTCfgFileName, "");

  sprintf(MigDirName, "");
  sprintf(MigFileName, "");
  sprintf(Header1FileName, "");
  sprintf(Header2FileName, "");
  sprintf(QCFileName, "");
  sprintf(RestartFileName, "");
  sprintf(VecMigM1FileName, "");
  sprintf(VecMigM2FileName, "");
  sprintf(VecMigM3FileName, "");
  sprintf(VecMigQualityFileName, "");
  sprintf(InlineFileName, "");
  sprintf(XlineFileName, "");
  sprintf(ConfidenceFileName, "");

  // set all switches to false
  cdp_taper = false;
  offset_innermute = false;
  offset_outermute = false;
  offset_outermute_linear = false;
  g_restart = false;
  g_Dip_Gather = false;
  g_No_Reflection_and_Headwave = false;
  frac = false;
  applytpow = false;
  AngleMigration = true;
  VectorMigration = false;
  VectorMigrationECED = false;
  DipCube = false;
  DipCubeConfidence = false;
  conditionAngleBins =false;
  trueAmplitudeModus = 1;
  UrsinRegularization = false;
  kinematicWeightsOnly = false;

  frequ1 = -1;
  frequ2 = -1;
  frequ3 = -1;
  frequ4 = -1;

  RayAperture_deg = 180.f;

  BlockMode = false;
//   // File types
//   TraceFileMode = SEGY_BIGENDIAN;
//   MigFileMode = SEGY_BIGENDIAN;

  // Number of Time Samples and Intervall in the Traces
//   g_NT = -1;
//   g_dT = 0;
}

AngleMigrationJob::~AngleMigrationJob()
{
  if (NTraceFiles > 0)
    {
      for (int iname = 0; iname < NTraceFiles; iname++)
	delete[] TraceFileName[iname];
      delete[] TraceFileName;
    }
}

AngleMigrationJob::AngleMigrationJob(const AngleMigrationJob& Job)
{
    memcpy(ProjectName, Job.ProjectName, 199);
    memcpy(JobName, Job.JobName, 199);
    LogLevel=Job.LogLevel;
    memcpy(LogFile, Job.LogFile, 199);

    BlockMode = Job.BlockMode;

    JobSize = Job.JobSize;

    geom = Job.geom;

    MigVol = Job.MigVol;

    DANG_RAYFAN_COARSE = Job.DANG_RAYFAN_COARSE;
    DANG_RAYFAN_FINE = Job.DANG_RAYFAN_FINE;
    DANG_RAYFAN_FILE = Job. DANG_RAYFAN_FILE;
    DANG_RAYFAN_DIP = Job.DANG_RAYFAN_DIP; 
  			  		      
    conditionAngleBins = Job.conditionAngleBins;

    RayAperture_deg = Job.RayAperture_deg;
    RayApertureTaperWidth_deg = Job.RayApertureTaperWidth_deg;

    CDPApertureMin_mtr = Job.CDPApertureMin_mtr;
    CDPAperture_deg = Job.CDPAperture_deg;
    CDPApertureTaperWidth_mtr = Job.CDPApertureTaperWidth_mtr;

    phi_open_start = Job.phi_open_start;
    phi_open_end = Job.phi_open_end;
    dphi_open = Job.dphi_open;
    N_PHI_OPEN = Job.N_PHI_OPEN;
    N_GATHER = Job.N_GATHER;

    dazimuth_deg = Job.dazimuth_deg;
    N_AZIMUTH_CLASSES = Job.N_AZIMUTH_CLASSES;

    theta_dip_start = Job.theta_dip_start;
    theta_dip_end = Job.theta_dip_end;
    g_DipTaperWidth = Job.g_DipTaperWidth;

    cdp_taper = Job.cdp_taper;
    cdpx_taperwidth = Job.cdpx_taperwidth;
    cdpy_taperwidth = Job.cdpy_taperwidth;

    offset_innermute = Job.offset_innermute;
    offset_innermute_offset = Job.offset_innermute_offset;
    offset_outermute = Job.offset_outermute;
    offset_outermute_linear = Job.offset_outermute_linear;
    offset_outermutetaperwidth = Job.offset_outermutetaperwidth;
    offset_outermute_offset1 = Job.offset_outermute_offset1;
    offset_outermute_t1 = Job.offset_outermute_t1;
    offset_outermute_offset2 = Job.offset_outermute_offset2;
    offset_outermute_t2 = Job.offset_outermute_t2;

    dSrcxmax = Job.dSrcxmax;
    dSrcymax = Job.dSrcymax;
    dRcvxmax = Job.dRcvxmax;
    dRcvymax = Job.dRcvymax;
    dRayxmax = Job.dRayxmax;
    dRayymax = Job.dRayymax;

    averageTraceDistance = Job.averageTraceDistance;

    NTraceFiles = Job.NTraceFiles;
    if (NTraceFiles > 0)
    {
	TraceFileName = new char*[NTraceFiles];
	for (int i = 0; i < NTraceFiles; i++)
	{
	    TraceFileName[i] = new char[199];
	    memcpy(TraceFileName[i], Job.TraceFileName[i], 199);
	}
    }

    memcpy(MigFileName, Job.MigFileName, 199);
    memcpy(QCFileName, Job.QCFileName, 199);
    memcpy(RestartFileName, Job.RestartFileName, 199);
    memcpy(Header1FileName, Job.Header1FileName, 199);
    memcpy(Header2FileName, Job.Header2FileName, 199);
    memcpy(MigDirName, Job.MigDirName, 199);

    memcpy(VecMigM1FileName, Job.VecMigM1FileName, 199);
    memcpy(VecMigM2FileName, Job.VecMigM2FileName, 199);
    memcpy(VecMigM3FileName, Job.VecMigM3FileName, 199);
    memcpy(VecMigQualityFileName, Job.VecMigQualityFileName, 199);

    memcpy(TTCfgFileName, Job.TTCfgFileName, 199);
    memcpy(TTDirName, Job.TTDirName, 199);
    memcpy(TTFilePrefix, Job.TTFilePrefix, 199);


    TTVol = Job.TTVol;

    TraceFileMode = Job.TraceFileMode;
    MigFileMode = Job.MigFileMode;

    IOMode = Job.IOMode;

    g_T0 = Job.g_T0;
    g_Tmin = Job.g_Tmin;
    g_Tmax = Job.g_Tmax;

    tEndTaperLength = Job.tEndTaperLength;

    TTMinDiff = Job.TTMinDiff;

    frequ1 = Job.frequ1;
    frequ2 = Job.frequ2;
    frequ3 = Job.frequ3;
    frequ4 = Job.frequ4;

    g_restart = Job.g_restart;
    g_Dip_Gather = Job.g_Dip_Gather;

    trueAmplitudeModus = Job.trueAmplitudeModus;

    UrsinRegularization = Job.UrsinRegularization;

    kinematicWeightsOnly = Job.kinematicWeightsOnly;

    g_No_Reflection_and_Headwave = Job.g_No_Reflection_and_Headwave;

    Pointsplit_distance = Job.Pointsplit_distance;
    MAXRAY = Job.MAXRAY;
    clip = Job.clip;
    frac = Job.frac;

    applytpow = Job.applytpow;
    tpow = Job.tpow;

    AngleMigration = Job.AngleMigration;
    VectorMigration = Job.VectorMigration;
    VectorMigrationECED = Job.VectorMigrationECED;

    DipCube = Job.DipCube;
    DipCubeConfidence = Job.DipCubeConfidence;
    memcpy(InlineFileName, Job.InlineFileName, 199);
    memcpy(XlineFileName, Job.XlineFileName, 199);
    memcpy(ConfidenceFileName, Job.ConfidenceFileName, 199);
    PropFileMode = Job.PropFileMode;


    DCVol = Job.DCVol;

    diprange_deg = Job.diprange_deg;
}

AngleMigrationJob& AngleMigrationJob::operator = (const AngleMigrationJob& Job)
{
    memcpy(ProjectName, Job.ProjectName, 199);
    memcpy(JobName, Job.JobName, 199);
    LogLevel=Job.LogLevel;
    memcpy(LogFile, Job.LogFile, 199);

    BlockMode = Job.BlockMode;

    JobSize = Job.JobSize;

    geom = Job.geom;

    MigVol = Job.MigVol;

    DANG_RAYFAN_COARSE = Job.DANG_RAYFAN_COARSE;
    DANG_RAYFAN_FINE = Job.DANG_RAYFAN_FINE;
    DANG_RAYFAN_FILE = Job. DANG_RAYFAN_FILE;
    DANG_RAYFAN_DIP = Job.DANG_RAYFAN_DIP; 

    conditionAngleBins = Job.conditionAngleBins;
  	
    RayAperture_deg = Job.RayAperture_deg;
    RayApertureTaperWidth_deg = Job.RayApertureTaperWidth_deg;

    CDPApertureMin_mtr = Job.CDPApertureMin_mtr;
    CDPAperture_deg = Job.CDPAperture_deg;
    CDPApertureTaperWidth_mtr = Job.CDPApertureTaperWidth_mtr;

    phi_open_start = Job.phi_open_start;
    phi_open_end = Job.phi_open_end;
    dphi_open = Job.dphi_open;
    N_PHI_OPEN = Job.N_PHI_OPEN;
    N_GATHER = Job.N_GATHER;

    dazimuth_deg = Job.dazimuth_deg;
    N_AZIMUTH_CLASSES = Job.N_AZIMUTH_CLASSES;

    theta_dip_start = Job.theta_dip_start;
    theta_dip_end = Job.theta_dip_end;
    g_DipTaperWidth = Job.g_DipTaperWidth;

    cdp_taper = Job.cdp_taper;
    cdpx_taperwidth = Job.cdpx_taperwidth;
    cdpy_taperwidth = Job.cdpy_taperwidth;

    offset_innermute = Job.offset_innermute;
    offset_innermute_offset = Job.offset_innermute_offset;
    offset_outermute = Job.offset_outermute;
    offset_outermute_linear = Job.offset_outermute_linear;
    offset_outermutetaperwidth = Job.offset_outermutetaperwidth;
    offset_outermute_offset1 = Job.offset_outermute_offset1;
    offset_outermute_t1 = Job.offset_outermute_t1;
    offset_outermute_offset2 = Job.offset_outermute_offset2;
    offset_outermute_t2 = Job.offset_outermute_t2;

    dSrcxmax = Job.dSrcxmax;
    dSrcymax = Job.dSrcymax;
    dRcvxmax = Job.dRcvxmax;
    dRcvymax = Job.dRcvymax;
    dRayxmax = Job.dRayxmax;
    dRayymax = Job.dRayymax;

    averageTraceDistance = Job.averageTraceDistance;

    if (NTraceFiles > 0)
    {
	for (int iname = 0; iname < NTraceFiles; iname++)
	    delete[] TraceFileName[iname];
	delete[] TraceFileName;
    }

    NTraceFiles = Job.NTraceFiles;
    if (NTraceFiles > 0)
    {
	TraceFileName = new char*[NTraceFiles];
	for (int i = 0; i < NTraceFiles; i++)
	{
	    TraceFileName[i] = new char[199];
	    memcpy(TraceFileName[i], Job.TraceFileName[i], 199);
	}
    }

    memcpy(MigFileName, Job.MigFileName, 199);
    memcpy(QCFileName, Job.QCFileName, 199);
    memcpy(RestartFileName, Job.RestartFileName, 199);
    memcpy(Header1FileName, Job.Header1FileName, 199);
    memcpy(Header2FileName, Job.Header2FileName, 199);
    memcpy(MigDirName, Job.MigDirName, 199);

    memcpy(VecMigM1FileName, Job.VecMigM1FileName, 199);
    memcpy(VecMigM2FileName, Job.VecMigM2FileName, 199);
    memcpy(VecMigM3FileName, Job.VecMigM3FileName, 199);
    memcpy(VecMigQualityFileName, Job.VecMigQualityFileName, 199);

    memcpy(TTCfgFileName, Job.TTCfgFileName, 199);
    memcpy(TTDirName, Job.TTDirName, 199);
    memcpy(TTFilePrefix, Job.TTFilePrefix, 199);


    TTVol = Job.TTVol;

    TraceFileMode = Job.TraceFileMode;
    MigFileMode = Job.MigFileMode;

    IOMode = Job.IOMode;

    g_T0 = Job.g_T0;
    g_Tmin = Job.g_Tmin;
    g_Tmax = Job.g_Tmax;

    tEndTaperLength = Job.tEndTaperLength;

    TTMinDiff = Job.TTMinDiff;
    RayApertureTaperWidth_deg = Job.RayApertureTaperWidth_deg;

    frequ1 = Job.frequ1;
    frequ2 = Job.frequ2;
    frequ3 = Job.frequ3;
    frequ4 = Job.frequ4;

    g_restart = Job.g_restart;
    g_Dip_Gather = Job.g_Dip_Gather;

    trueAmplitudeModus = Job.trueAmplitudeModus;

    UrsinRegularization = Job.UrsinRegularization;

    kinematicWeightsOnly = Job.kinematicWeightsOnly;

    g_No_Reflection_and_Headwave = Job.g_No_Reflection_and_Headwave;

    Pointsplit_distance = Job.Pointsplit_distance;

    MAXRAY = Job.MAXRAY;
    clip = Job.clip;
    frac = Job.frac;

    applytpow = Job.applytpow;
    tpow = Job.tpow;

    AngleMigration = Job.AngleMigration;
    VectorMigration = Job.VectorMigration;
    VectorMigrationECED = Job.VectorMigrationECED;

    DipCube = Job.DipCube;
    DipCubeConfidence = Job.DipCubeConfidence;
    memcpy(InlineFileName, Job.InlineFileName, 199);
    memcpy(XlineFileName, Job.XlineFileName, 199);
    memcpy(ConfidenceFileName, Job.ConfidenceFileName, 199);
    PropFileMode = Job.PropFileMode;


    DCVol = Job.DCVol;

    diprange_deg = Job.diprange_deg;

    return *this;
}
